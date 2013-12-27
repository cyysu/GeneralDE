#include "cpe/pal/pal_stdio.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/tl/tl_manage.h"
#include "gd/app/app_context.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_require.h"
#include "usf/mongo_driver/mongo_pkg.h"
#include "friend_svr_ops.h"

int friend_svr_db_send_query(friend_svr_t svr, logic_require_t require, uint64_t user_id) {
    int pkg_r;
    mongo_pkg_t db_pkg;

    db_pkg = mongo_cli_proxy_pkg_buf(svr->m_db);
    if (db_pkg == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: %s: get db pkg fail!",
            friend_svr_name(svr), logic_require_name(require));
        return -1;
    }

    mongo_pkg_init(db_pkg);
    mongo_pkg_set_collection(db_pkg, "friend_data");
    mongo_pkg_set_op(db_pkg, mongo_db_op_query);

    pkg_r = 0;

    pkg_r |= mongo_pkg_doc_open(db_pkg);
    pkg_r |= mongo_pkg_append_int64(db_pkg, "user_id", (int64_t)user_id);
    pkg_r |=  mongo_pkg_doc_close(db_pkg);

    /*需要的列 */
    pkg_r |= mongo_pkg_doc_open(db_pkg);
    pkg_r |= mongo_pkg_append_int32(db_pkg, dr_entry_name(svr->m_record_fuid_entry), 1);
    pkg_r |= mongo_pkg_doc_close(db_pkg);

    if (pkg_r) {
        CPE_ERROR(
            svr->m_em, "%s: %s: build query req fail!",
            friend_svr_name(svr), logic_require_name(require));
        return -1;
    }

    if (mongo_cli_proxy_send(svr->m_db, db_pkg, require, svr->m_record_list_meta, 32, NULL, NULL, 0) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: %s: send db request fail!",
            friend_svr_name(svr), logic_require_name(require));
        return -1;
    }

    return 0;
}

int friend_svr_db_send_insert(friend_svr_t svr, logic_require_t require, void const * record) {
    mongo_pkg_t db_pkg;
    int pkg_r;

    db_pkg = mongo_cli_proxy_pkg_buf(svr->m_db);
    if (db_pkg == NULL) {
        CPE_ERROR(svr->m_em, "%s: %s:: get db pkg fail!", friend_svr_name(svr), logic_require_name(require));
        return -1;
    }

    mongo_pkg_init(db_pkg);
    mongo_pkg_set_collection(db_pkg, "friend_data");
    mongo_pkg_set_op(db_pkg, mongo_db_op_insert);

    pkg_r = 0;
    pkg_r |= mongo_pkg_doc_append(db_pkg, svr->m_record_meta, record, svr->m_record_size);

    if (pkg_r) {
        CPE_ERROR(
            svr->m_em, "%s: %s: build insert req fail!",
            friend_svr_name(svr), logic_require_name(require));
        return -1;
    }

    if (mongo_cli_proxy_send(svr->m_db, db_pkg, require, svr->m_record_list_meta, 0, NULL, NULL, 0) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: %s: send db request fail!",
            friend_svr_name(svr), logic_require_name(require));
        return -1;
    }

    return 0;
}

int friend_svr_db_send_remove(friend_svr_t svr, logic_require_t require, uint64_t uid, uint64_t fuid) {
    mongo_pkg_t db_pkg;
    int pkg_r;
    char buf[64];

    db_pkg = mongo_cli_proxy_pkg_buf(svr->m_db);
    if (db_pkg == NULL) {
        CPE_ERROR(svr->m_em, "%s: %s:: get db pkg fail!", friend_svr_name(svr), logic_require_name(require));
        return -1;
    }

    mongo_pkg_init(db_pkg);
    mongo_pkg_set_collection(db_pkg, "friend_data");
    mongo_pkg_set_op(db_pkg, mongo_db_op_delete);

    pkg_r = 0;

    pkg_r |= mongo_pkg_doc_open(db_pkg);
    snprintf(buf, sizeof(buf), FMT_UINT64_T"-"FMT_UINT64_T, uid, fuid);
    pkg_r |= mongo_pkg_append_string(db_pkg, "_id", buf);
    pkg_r |=  mongo_pkg_doc_close(db_pkg);

    if (pkg_r) {
        CPE_ERROR(
            svr->m_em, "%s: %s: build delete req fail!",
            friend_svr_name(svr), logic_require_name(require));
        return -1;
    }

    if (mongo_cli_proxy_send(svr->m_db, db_pkg, require, svr->m_record_list_meta, 0, NULL, NULL, 0) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: %s: send db request fail!",
            friend_svr_name(svr), logic_require_name(require));
        return -1;
    }

    return 0;
}


