#include <assert.h>
#include "cpe/dp/dp_manage.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_bson.h"
#include "usf/logic/logic_require.h"
#include "usf/logic_use/logic_require_queue.h"
#include "usf/logic_use/logic_data_dyn.h"
#include "usf/logic_use/logic_uni_res.h"
#include "usf/mongo_driver/mongo_pkg.h"
#include "usf/mongo_cli/mongo_cli_proxy.h"
#include "protocol/mongo_cli/mongo_cli.h"
#include "mongo_cli_internal_ops.h"

int mongo_cli_proxy_send(
    mongo_cli_proxy_t agent, mongo_pkg_t pkg, logic_require_t require,
    LPDRMETA result_meta, int result_count_init)
{
    if (agent->m_outgoing_send_to == NULL) {
        CPE_INFO(
            agent->m_em, "%s: send: no outgoing_send_to configured",
            mongo_cli_proxy_name(agent));
        goto SEND_ERROR;
    }

    /*查询请求需要设置好接受返回数据的结构 */
    if (mongo_pkg_op(pkg) == mongo_db_op_query || mongo_pkg_op(pkg) == mongo_db_op_get_more) {
        if (require == NULL) {
            CPE_ERROR(agent->m_em, "%s: send_request: query operation no require!",mongo_cli_proxy_name(agent));
            goto SEND_ERROR;
        }

        if (result_meta == NULL) {
            CPE_ERROR(agent->m_em, "%s: send_request: query operation no result meta!",mongo_cli_proxy_name(agent));
            goto SEND_ERROR;
        }

        if (logic_uni_res_init(require, result_meta, result_count_init) != 0) {
			CPE_ERROR(
				agent->m_em, "%s: send_request: init result meta fail, meta=%s, init-count=%d!",
				mongo_cli_proxy_name(agent), dr_meta_name(result_meta), result_count_init);
            goto SEND_ERROR;
		}

        mongo_pkg_set_id(pkg, logic_require_id(require));
    }

    if (dp_dispatch_by_string(agent->m_outgoing_send_to, mongo_pkg_to_dp_req(pkg), agent->m_em) != 0) {
        CPE_INFO(agent->m_em, "%s: send_request: dispatch return fail!", mongo_cli_proxy_name(agent));
        goto SEND_ERROR;
    }

    if (require) {
        /*查询请求有响应，其他请求需要单独获取响应 */
        if (mongo_pkg_op(pkg) != mongo_db_op_query && mongo_pkg_op(pkg) != mongo_db_op_get_more) {
            mongo_pkg_t cmd_pkg = mongo_cli_proxy_cmd_buf(agent);

            mongo_pkg_cmd_init(cmd_pkg, mongo_pkg_ns(pkg));

            mongo_pkg_set_id(cmd_pkg, logic_require_id(require));

            mongo_pkg_doc_open(cmd_pkg);
            mongo_pkg_append_int32(cmd_pkg, "getlasterror", 1);
            mongo_pkg_doc_close(cmd_pkg);

            if (logic_uni_res_init(require, agent->m_meta_lasterror, 1) != 0) {
                CPE_ERROR(agent->m_em, "%s: send_request: init lasterror meta fail!", mongo_cli_proxy_name(agent));
                goto SEND_ERROR;
            }

            if (dp_dispatch_by_string(agent->m_outgoing_send_to, mongo_pkg_to_dp_req(cmd_pkg), agent->m_em) != 0) {
                CPE_INFO(agent->m_em, "%s: send_request: dispatch getLastError return fail!", mongo_cli_proxy_name(agent));
                goto SEND_ERROR;
            }
        }

        if (logic_require_queue_add(agent->m_require_queue, logic_require_id(require)) != 0) {
            CPE_ERROR(agent->m_em, "%s: send_request: save require id fail!", mongo_cli_proxy_name(agent));
            goto SEND_ERROR;
        }
    }

    return 0;

SEND_ERROR:
    if (require) logic_require_error(require);
    return -1;
}

int mongo_cli_proxy_recv(dp_req_t req, void * ctx, error_monitor_t em) {
    struct mongo_cli_proxy * proxy;
    mongo_pkg_t pkg;
    uint32_t sn;
    logic_require_t require;

    proxy = (struct mongo_cli_proxy *)ctx;

    pkg = mongo_pkg_from_dp_req(req);
    if (pkg == NULL) {
        CPE_ERROR(em, "%s: cast to pkg fail!", mongo_cli_proxy_name(proxy));
        return -1;
    }

    sn = mongo_pkg_response_to(pkg);
    require = logic_require_queue_remove_get(proxy->m_require_queue, sn);
    if (require == NULL) {
        CPE_ERROR(em, "%s: require %d not exist in queue!", mongo_cli_proxy_name(proxy), sn);
        return -1;
    }

    if (mongo_pkg_op(pkg) == mongo_db_op_replay) {
        struct mongo_doc_it doc_it;
        logic_data_t result_data;
        LPDRMETA result_meta;
        mongo_doc_t doc;

        result_data = logic_uni_res_data(require);
        if (result_data == NULL) {
            CPE_ERROR(proxy->m_em, "%s: recv_response: get result data fail!", mongo_cli_proxy_name(proxy));
            logic_require_set_error(require);
            return -1;
        }

        result_meta = logic_data_record_meta(result_data);
        if (result_meta == NULL) {
            CPE_ERROR(proxy->m_em, "%s: recv_response: get result meta fail!", mongo_cli_proxy_name(proxy));
            logic_require_set_error(require);
            return -1;
        }

        mongo_pkg_doc_it(&doc_it, pkg);
        while((doc = mongo_pkg_doc_it_next(&doc_it))) {
            void * result = logic_data_record_append(result_data);
            if (result == NULL) {
                CPE_ERROR(
                    proxy->m_em, "%s: recv_response: append record fail, record count is %d!",
                    mongo_cli_proxy_name(proxy), (int)logic_data_record_count(result_data));
                logic_require_set_error(require);
                return -1;
            }

            if (dr_bson_read(result, dr_meta_size(result_meta), mongo_doc_data(doc), mongo_doc_size(doc), result_meta, proxy->m_em) < 0) {
                CPE_ERROR(proxy->m_em, "%s: recv_response: bson read fail!", mongo_cli_proxy_name(proxy));
                logic_require_set_error(require);
                return -1;
            }
        }

        if (result_meta == proxy->m_meta_lasterror) {
            if (logic_data_record_count(result_data) != 1) {
                CPE_ERROR(proxy->m_em, "%s: recv_response: getlasterror: result count error!", mongo_cli_proxy_name(proxy));
                logic_require_set_error(require);
                return -1;
            }
            else {
                MONGO_LASTERROR * lasterror = (MONGO_LASTERROR *)logic_data_record_at(result_data, 0);
                if (lasterror->code == 0) {
                    if (proxy->m_debug) {
                        CPE_INFO(proxy->m_em, "%s: recv_response: req %d: error: ok", mongo_cli_proxy_name(proxy), sn);
                    }
                }
                else {
                    if (proxy->m_debug) {
                        CPE_INFO(
                            proxy->m_em, "%s: recv_response: req %d: error: %d %s",
                            mongo_cli_proxy_name(proxy), sn, lasterror->code, lasterror->err);
                    }

                    logic_require_set_error_ex(require, lasterror->code);
                    return 0;
                }
            }
        }
    }

    logic_require_set_done(require);
    return 0;
}
