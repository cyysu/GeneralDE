#include <assert.h>
#include "cpe/dp/dp_manage.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_bson.h"
#include "usf/logic/logic_require.h"
#include "usf/logic/logic_data.h"
#include "usf/logic_use/logic_require_queue.h"
#include "usf/logic_use/logic_data_dyn.h"
#include "usf/logic_use/logic_uni_res.h"
#include "usf/mongo_driver/mongo_pkg.h"
#include "usf/mongo_cli/mongo_cli_proxy.h"
#include "protocol/mongo_cli/mongo_cli.h"
#include "mongo_cli_internal_ops.h"

static int mongo_cli_proxy_send_check_save_result_build_info(
    mongo_cli_proxy_t agent, logic_require_t require, const char * result_prefix)
{
    MONGO_RESULT_BUILD_INFO * result_build_info;
    logic_data_t result_build_info_data;

    if (result_prefix == NULL) return 0;

    result_build_info_data = 
        logic_require_data_get_or_create(
            require, agent->m_meta_result_build_info, dr_meta_size(agent->m_meta_result_build_info));
    if (result_build_info_data == NULL) {
        CPE_ERROR(agent->m_em, "%s: send_request: init result_build_info meta fail!", mongo_cli_proxy_name(agent));
        return -1;
    }

    result_build_info = (MONGO_RESULT_BUILD_INFO*)logic_data_data(result_build_info_data);

    strncpy(result_build_info->prefix, result_prefix, sizeof(result_build_info->prefix));

    return 0;
}

static int mongo_cli_proxy_send_check_save_cmd(mongo_cli_proxy_t agent, mongo_pkg_t pkg, logic_require_t require) {
    bson_iterator bson_it;
    MONGO_CMD_INFO * cmd_info;
    logic_data_t cmd_info_data;

    if (strcmp(mongo_pkg_collection(pkg), "$cmd") != 0) return 0;

    cmd_info_data = 
        logic_require_data_get_or_create(
            require, agent->m_meta_cmd_info, dr_meta_size(agent->m_meta_cmd_info));
    if (cmd_info_data == NULL) {
        CPE_ERROR(agent->m_em, "%s: send_request: init cmd_info meta fail!", mongo_cli_proxy_name(agent));
        return -1;
    }

    cmd_info = (MONGO_CMD_INFO*)logic_data_data(cmd_info_data);

    if (mongo_pkg_find(&bson_it, pkg, 0, "findandmodify") == 0) {
        cmd_info->cmd = MONGO_CMD_FINDANDMODIFY;
    }

    return 0;
}

int mongo_cli_proxy_send(
    mongo_cli_proxy_t agent, mongo_pkg_t pkg, logic_require_t require,
    LPDRMETA result_meta, int result_count_init, const char * result_prefix)
{
    if (agent->m_outgoing_send_to == NULL) {
        CPE_ERROR(
            agent->m_em, "%s: send: no outgoing_send_to configured",
            mongo_cli_proxy_name(agent));
        goto SEND_ERROR;
    }

    if (strcmp(mongo_pkg_db(pkg), "") == 0) {
        if (agent->m_dft_db[0] == 0) {
            CPE_ERROR(agent->m_em, "%s: send: sending pkg no db name!", mongo_cli_proxy_name(agent));
            goto SEND_ERROR;
        }

        if (mongo_pkg_set_db(pkg, agent->m_dft_db) != 0) {
            CPE_ERROR(
                agent->m_em, "%s: send: sending pkg set dft db %s fail!",
                mongo_cli_proxy_name(agent), agent->m_dft_db);
            goto SEND_ERROR;
        }
    }

    if (mongo_cli_proxy_send_check_save_result_build_info(agent, require, result_prefix) != 0) {
        CPE_ERROR(agent->m_em, "%s: send: save result_build_info error!", mongo_cli_proxy_name(agent));
        goto SEND_ERROR;
    }

    /*查询请求需要设置好接受返回数据的结构 */
    if (mongo_pkg_op(pkg) == mongo_db_op_query || mongo_pkg_op(pkg) == mongo_db_op_get_more) {
        if (require) {
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

            if (mongo_cli_proxy_send_check_save_cmd(agent, pkg, require) != 0) goto SEND_ERROR;
        }
    }

    if (dp_dispatch_by_string(agent->m_outgoing_send_to, mongo_pkg_to_dp_req(pkg), agent->m_em) != 0) {
        CPE_INFO(agent->m_em, "%s: send_request: dispatch return fail!", mongo_cli_proxy_name(agent));
        goto SEND_ERROR;
    }

    if (require) {
        /*查询请求有响应，其他请求需要单独获取响应 */
        if (mongo_pkg_op(pkg) != mongo_db_op_query && mongo_pkg_op(pkg) != mongo_db_op_get_more) {
            mongo_pkg_t cmd_pkg = mongo_cli_proxy_cmd_buf(agent);

            mongo_pkg_cmd_init(cmd_pkg);
            mongo_pkg_set_db(cmd_pkg, mongo_pkg_db(pkg));

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
