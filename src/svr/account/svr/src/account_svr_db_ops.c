#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_bson.h"
#include "gd/app/app_context.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_require.h"
#include "usf/mongo_driver/mongo_pkg.h"
#include "account_svr_ops.h"

int account_svr_db_send_query_by_logic_id(account_svr_t svr, logic_require_t require, SVR_ACCOUNT_LOGIC_ID const * logic_id, LPDRMETA result_meta) {
    int pkg_r;
    mongo_pkg_t db_pkg;
    int i;
    LPDRMETA record_meta;

    assert(result_meta);

    if (result_meta == svr->m_meta_record_full_list) {
        record_meta = svr->m_meta_record_full;
    }
    else if (result_meta == svr->m_meta_record_basic_list) {
        record_meta = svr->m_meta_record_basic;
    }
    else {
        CPE_ERROR(svr->m_em, "%s: %s: result meta is unknown!", account_svr_name(svr), logic_require_name(require));
        return -1;
    }
    assert(record_meta != NULL);

    db_pkg = mongo_cli_proxy_pkg_buf(svr->m_db);
    if (db_pkg == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: %s: get db pkg fail!",
            account_svr_name(svr), logic_require_name(require));
        return -1;
    }

    mongo_pkg_init(db_pkg);
    mongo_pkg_set_collection(db_pkg, "account");
    mongo_pkg_set_op(db_pkg, mongo_db_op_query);

    pkg_r = 0;

    pkg_r |= mongo_pkg_doc_open(db_pkg);
    switch(logic_id->account_type) {
    case SVR_ACCOUNT_EMAIL:
        pkg_r |= mongo_pkg_append_string(db_pkg, "logic_email", logic_id->account);
        break;
    case SVR_ACCOUNT_MAC:
        pkg_r |= mongo_pkg_append_string(db_pkg, "logic_mac", logic_id->account);
        break;
    default:
        CPE_ERROR(
            svr->m_em, "%s: %s: unknown account type %d!",
            account_svr_name(svr), logic_require_name(require), logic_id->account_type);
        return -1;
    }
    pkg_r |=  mongo_pkg_doc_close(db_pkg);

    /*需要的列 */
    if (result_meta) {
        pkg_r |= mongo_pkg_doc_open(db_pkg);
        for(i = 0; i < dr_meta_entry_num(result_meta); ++i) {
            const char * entry_name = dr_entry_name(dr_meta_entry_at(record_meta, i));
            if (strcmp(entry_name, "account_id") == 0) continue;
            pkg_r |= mongo_pkg_append_int32(db_pkg, entry_name, 1);
        }
        pkg_r |= mongo_pkg_doc_close(db_pkg);
    }

    if (pkg_r) {
        CPE_ERROR(
            svr->m_em, "%s: %s: build query req fail!",
            account_svr_name(svr), logic_require_name(require));
        return -1;
    }

    if (mongo_cli_proxy_send(svr->m_db, db_pkg, require, result_meta, 1, NULL, NULL, NULL) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: %s: send db request fail!",
            account_svr_name(svr), logic_require_name(require));
        return -1;
    }

    return 0;
}

int account_svr_db_send_insert(
    account_svr_t svr, logic_require_t require,
    uint64_t account_id, const char * passwd, SVR_ACCOUNT_LOGIC_ID const * logic_id)
{
    mongo_pkg_t db_pkg;
    int pkg_r;

    db_pkg = mongo_cli_proxy_pkg_buf(svr->m_db);
    if (db_pkg == NULL) {
        CPE_ERROR(svr->m_em, "%s: %s:: get db pkg fail!", account_svr_name(svr), logic_require_name(require));
        return -1;
    }

    mongo_pkg_init(db_pkg);
    mongo_pkg_set_collection(db_pkg, "account");
    mongo_pkg_set_op(db_pkg, mongo_db_op_insert);

    pkg_r = 0;

    pkg_r |= mongo_pkg_doc_open(db_pkg);

    pkg_r |= mongo_pkg_append_int64(db_pkg, "_id", (int64_t)account_id);
    pkg_r |= mongo_pkg_append_string(db_pkg, "password", passwd);

    switch(logic_id->account_type) {
    case SVR_ACCOUNT_EMAIL:
        pkg_r |= mongo_pkg_append_string(db_pkg, "logic_email", logic_id->account);
        break;
    case SVR_ACCOUNT_MAC:
        pkg_r |= mongo_pkg_append_string(db_pkg, "logic_mac", logic_id->account);
        break;
    default:
        CPE_ERROR(
            svr->m_em, "%s: %s: unknown account type %d!",
            account_svr_name(svr), logic_require_name(require), logic_id->account_type);
        return -1;
    }
    pkg_r |=  mongo_pkg_doc_close(db_pkg);

    if (pkg_r) {
        CPE_ERROR(
            svr->m_em, "%s: %s: build insert req fail!",
            account_svr_name(svr), logic_require_name(require));
        return -1;
    }

    if (mongo_cli_proxy_send(svr->m_db, db_pkg, require, NULL, 0, NULL, NULL, NULL) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: %s: send db request fail!",
            account_svr_name(svr), logic_require_name(require));
        return -1;
    }

    return 0;
}

int account_svr_db_send_bind(
    account_svr_t svr, logic_require_t require, 
    uint64_t account_id, SVR_ACCOUNT_LOGIC_ID const * logic_id)
{
    mongo_pkg_t db_pkg;
    int pkg_r;

    db_pkg = mongo_cli_proxy_pkg_buf(svr->m_db);
    if (db_pkg == NULL) {
        CPE_ERROR(svr->m_em, "%s: %s:: get db pkg fail!", account_svr_name(svr), logic_require_name(require));
        return -1;
    }

    mongo_pkg_init(db_pkg);
    mongo_pkg_set_collection(db_pkg, "account");
    mongo_pkg_set_op(db_pkg, mongo_db_op_update);

    pkg_r = 0;

    pkg_r |= mongo_pkg_doc_open(db_pkg);

    pkg_r |= mongo_pkg_doc_open(db_pkg);
    pkg_r |= mongo_pkg_append_int64(db_pkg, "_id", (int64_t)account_id);
    pkg_r |=  mongo_pkg_doc_close(db_pkg);

    pkg_r |= mongo_pkg_append_start_object(db_pkg, "$set");
    switch(logic_id->account_type) {
    case SVR_ACCOUNT_EMAIL:
        pkg_r |= mongo_pkg_append_string(db_pkg, "logic_email", logic_id->account);
        break;
    case SVR_ACCOUNT_MAC:
        pkg_r |= mongo_pkg_append_string(db_pkg, "logic_mac", logic_id->account);
        break;
    default:
        CPE_ERROR(
            svr->m_em, "%s: %s: unknown account type %d!",
            account_svr_name(svr), logic_require_name(require), logic_id->account_type);
        return -1;
    }
    pkg_r |=  mongo_pkg_doc_close(db_pkg);

    if (pkg_r) {
        CPE_ERROR(
            svr->m_em, "%s: %s: build bind update req fail!",
            account_svr_name(svr), logic_require_name(require));
        return -1;
    }

    if (mongo_cli_proxy_send(svr->m_db, db_pkg, require, NULL, 0, NULL, NULL, NULL) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: %s: send db request fail!",
            account_svr_name(svr), logic_require_name(require));
        return -1;
    }

    return 0;
}

int account_svr_db_send_unbind(
    account_svr_t svr, logic_require_t require, 
    uint64_t account_id, uint16_t account_type)
{
    mongo_pkg_t db_pkg;
    int pkg_r;

    db_pkg = mongo_cli_proxy_pkg_buf(svr->m_db);
    if (db_pkg == NULL) {
        CPE_ERROR(svr->m_em, "%s: %s:: get db pkg fail!", account_svr_name(svr), logic_require_name(require));
        return -1;
    }

    mongo_pkg_init(db_pkg);
    mongo_pkg_set_collection(db_pkg, "account");
    mongo_pkg_set_op(db_pkg, mongo_db_op_update);

    pkg_r = 0;

    pkg_r |= mongo_pkg_doc_open(db_pkg);

    pkg_r |= mongo_pkg_doc_open(db_pkg);
    pkg_r |= mongo_pkg_append_int64(db_pkg, "_id", (int64_t)account_id);
    pkg_r |=  mongo_pkg_doc_close(db_pkg);

    pkg_r |= mongo_pkg_append_start_object(db_pkg, "$unset");
    switch(account_type) {
    case SVR_ACCOUNT_EMAIL:
        pkg_r |= mongo_pkg_append_string(db_pkg, "logic_email", "");
        break;
    case SVR_ACCOUNT_MAC:
        pkg_r |= mongo_pkg_append_string(db_pkg, "logic_mac", "");
        break;
    default:
        CPE_ERROR(
            svr->m_em, "%s: %s: unknown account type %d!",
            account_svr_name(svr), logic_require_name(require), account_type);
        return -1;
    }
    pkg_r |=  mongo_pkg_doc_close(db_pkg);

    if (pkg_r) {
        CPE_ERROR(
            svr->m_em, "%s: %s: build unbind update req fail!",
            account_svr_name(svr), logic_require_name(require));
        return -1;
    }

    if (mongo_cli_proxy_send(svr->m_db, db_pkg, require, NULL, 0, NULL, NULL, NULL) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: %s: send db request fail!",
            account_svr_name(svr), logic_require_name(require));
        return -1;
    }

    return 0;
}