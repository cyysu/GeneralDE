#include <assert.h>
#include "cpe/pal/pal_platform.h"
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

static int mongo_cli_proxy_recv_build_results(
    mongo_cli_proxy_t proxy, mongo_pkg_t pkg, logic_data_t result_data, LPDRMETA result_meta)
{
    struct mongo_doc_it doc_it;
    mongo_doc_t doc;

    mongo_pkg_doc_it(&doc_it, pkg);
    while((doc = mongo_pkg_doc_it_next(&doc_it))) {
        void * result = logic_data_record_append(result_data);
        if (result == NULL) {
            CPE_ERROR(
                proxy->m_em, "%s: recv_response: append record fail, record count is %d!",
                mongo_cli_proxy_name(proxy), (int)logic_data_record_count(result_data));
            return -1;
        }

        if (dr_bson_read(result, dr_meta_size(result_meta), mongo_doc_data(doc), mongo_doc_size(doc), result_meta, proxy->m_em) < 0) {
            CPE_ERROR(proxy->m_em, "%s: recv_response: bson read fail!", mongo_cli_proxy_name(proxy));
            return -1;
        }
    }

    return 0;
}

static int mongo_cli_proxy_recv_build_result_from_it(mongo_cli_proxy_t proxy, bson_iterator * bson_it, logic_data_t result_data, LPDRMETA result_meta) {
    void * result;
    int32_t len;
    const char * value;
    char type = bson_iterator_type(bson_it);

    if (type != BSON_OBJECT) {
        CPE_ERROR(proxy->m_em, "%s: recv_response: find_and_modify: not support bson type %d!", mongo_cli_proxy_name(proxy), type);
        return -1;
    }

    result = logic_data_record_append(result_data);
    if (result == NULL) {
        CPE_ERROR(
            proxy->m_em, "%s: recv_response: append record fail, record count is %d!",
            mongo_cli_proxy_name(proxy), (int)logic_data_record_count(result_data));
        return -1;
    }

    value = bson_iterator_value(bson_it);
    CPE_COPY_HTON32(&len, value);

    if (dr_bson_read(result, dr_meta_size(result_meta), value, len, result_meta, proxy->m_em) < 0) {
        CPE_ERROR(proxy->m_em, "%s: recv_response: bson read fail, len=%d!", mongo_cli_proxy_name(proxy), len);
        return -1;
    }

    return 0;
}

static int mongo_cli_proxy_recv_process_find_and_modify(
    mongo_cli_proxy_t proxy, mongo_pkg_t pkg, logic_require_t require, logic_data_t result_data, LPDRMETA result_meta)
{
    bson_iterator bson_it;

    if (mongo_pkg_find(&bson_it, pkg, 0, "ok") != 0) {
        if (mongo_pkg_find(&bson_it, pkg, 0, "code") != 0) {
            int32_t err = (int32_t)bson_iterator_int(&bson_it);
            if (proxy->m_debug) {
                CPE_INFO(proxy->m_em, "%s: recv_response: find_and_modify: error: %d!", mongo_cli_proxy_name(proxy), err);
            }
            logic_require_set_error_ex(require, err);
            return -1;
        }
        else {
            if (proxy->m_debug) {
                CPE_INFO(proxy->m_em, "%s: recv_response: find_and_modify: error: (no error code)!", mongo_cli_proxy_name(proxy));
            }
            logic_require_set_error(require);
            return -1;
        }
    }

    if (mongo_pkg_find(&bson_it, pkg, 0, "value") != 0) {
        CPE_ERROR(proxy->m_em, "%s: recv_response: find_and_modify: find value fail!", mongo_cli_proxy_name(proxy));
        logic_require_error(require);
        return -1;
    }

    switch(bson_iterator_type(&bson_it)) {
    case BSON_NULL:
        break;
    case BSON_OBJECT:
        if (mongo_cli_proxy_recv_build_result_from_it(proxy, &bson_it, result_data, result_meta) != 0) {
            logic_require_error(require);
            return -1;
        }
        break;
    case BSON_ARRAY: {
        bson_iterator bson_sub_it;
        bson_iterator_subiterator(&bson_it, &bson_sub_it);

        while(bson_iterator_next(&bson_sub_it)) {
            if (mongo_cli_proxy_recv_build_result_from_it(proxy, &bson_sub_it, result_data, result_meta) != 0) {
                logic_require_error(require);
                return -1;
            }
        }
        break;
    }
    default:
        CPE_ERROR(
            proxy->m_em, "%s: recv_response: find_and_modify: value type %d error!", 
            mongo_cli_proxy_name(proxy), bson_iterator_type(&bson_it));
        logic_require_error(require);
        return -1;
    }

    logic_require_set_done(require);
    return 0;
}

static int mongo_cli_proxy_recv_process_find_last_error(
    mongo_cli_proxy_t proxy, mongo_pkg_t pkg, logic_require_t require, logic_data_t result_data)
{
    if (logic_data_record_count(result_data) != 1) {
        CPE_ERROR(proxy->m_em, "%s: recv_response: getlasterror: result count error!", mongo_cli_proxy_name(proxy));
        logic_require_set_error(require);
        return -1;
    }
    else {
        MONGO_LASTERROR * lasterror = (MONGO_LASTERROR *)logic_data_record_at(result_data, 0);
        if (lasterror->code == 0) {
            if (proxy->m_debug) {
                CPE_INFO(proxy->m_em, "%s: recv_response: req %d: ok", mongo_cli_proxy_name(proxy), logic_require_id(require));
            }
        }
        else {
            if (proxy->m_debug) {
                CPE_INFO(
                    proxy->m_em, "%s: recv_response: req %d: error: %d %s",
                    mongo_cli_proxy_name(proxy), logic_require_id(require), lasterror->code, lasterror->err);
            }

            logic_require_set_error_ex(require, lasterror->code);
            return 0;
        }
    }

    logic_require_set_done(require);
    return 0;
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
        logic_data_t cmd_info_data;
        MONGO_CMD_INFO* cmd_info;
        logic_data_t result_data;
        LPDRMETA result_meta;

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

        cmd_info_data = logic_require_data_find(require, "mongo_cmd_info");
        cmd_info = cmd_info_data ? (MONGO_CMD_INFO*)logic_data_data(cmd_info_data) : NULL;

        if (cmd_info && cmd_info->cmd == MONGO_CMD_FINDANDMODIFY) {
            return mongo_cli_proxy_recv_process_find_and_modify(proxy, pkg, require, result_data, result_meta);
        }
        else {
            if (mongo_cli_proxy_recv_build_results(proxy, pkg, result_data, result_meta) != 0) {
                logic_require_set_error(require);
                return -1;
            }

            if (result_meta == proxy->m_meta_lasterror) {
                return mongo_cli_proxy_recv_process_find_last_error(proxy, pkg, require, result_data);
            }
        }
    }

    logic_require_set_done(require);
    return 0;
}
