#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "usf/bpg_pkg/bpg_pkg.h"
#include "usf/logic/logic_manage.h"
#include "usf/logic/logic_require.h"
#include "usf/logic/logic_data.h"
#include "usf/bpg_cli/bpg_cli_proxy.h"
#include "bpg_cli_internal_types.h"
#include "protocol/bpg_cli_pkg_info.h"

extern unsigned char g_metalib_bpg_cli_pkg_info[];

static int bpg_cli_proxy_save_pkg_info(logic_require_t require, bpg_pkg_t pkg, error_monitor_t em) {
    logic_data_t data;
    LPDRMETA meta;
    BPG_CLI_PKG_INFO * pkg_info;

    meta = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_bpg_cli_pkg_info, "bpg_cli_pkg_info");
    if (meta == NULL) {
        CPE_ERROR(em, "bpg_cli_proxy_rsp: save_pkg_info: bpg_cli_pkg_info exist in metalib!");
        return -1;
    }

    data = logic_require_data_get_or_create(require, meta, 0);
    if (data == NULL) {
        CPE_ERROR(em, "bpg_cli_proxy_rsp: save_pkg_info: crate data_head fail!");
        return -1;
    }

    pkg_info = (BPG_CLI_PKG_INFO *)logic_data_data(data);
    pkg_info->cmd = bpg_pkg_cmd(pkg);
    pkg_info->errorNo = bpg_pkg_errno(pkg);

    return 0;
}

static int bpg_cli_proxy_save_main_body(logic_require_t require, bpg_pkg_t pkg, error_monitor_t em) {
    logic_data_t data;
    LPDRMETA meta;
    size_t size;

    meta = bpg_pkg_main_data_meta(pkg, em);
    if (meta == NULL) return 0;

    data = logic_require_data_get_or_create(require, meta, bpg_pkg_body_origin_len(pkg));
    if (data == NULL) {
        CPE_ERROR(em, "bpg_cli_proxy_rsp: save_main_body: crate data with meta %s fail!", dr_meta_name(meta));
        return -1;
    }

    size = logic_data_capacity(data);
    if (bpg_pkg_get_main_data(pkg, logic_data_meta(data), logic_data_data(data), &size, em) != 0) {
        CPE_ERROR(em, "bpg_cli_proxy_rsp: save_main_body: get main data fail!");
        return -1;
    }

    return 0;
}

static int bpg_cli_proxy_save_append_infos(logic_require_t require, bpg_pkg_t pkg, error_monitor_t em) {
    logic_data_t data;
    LPDRMETA meta;
    size_t size;
    int32_t append_info_count;
    int32_t i;

    append_info_count = bpg_pkg_append_info_count(pkg);
    for(i = 0; i < append_info_count; ++i) {
        bpg_pkg_append_info_t append_info = bpg_pkg_append_info_at(pkg, i);
        assert(append_info);

        meta = bpg_pkg_append_data_meta(pkg, append_info, em);
        if (meta == NULL) {
            CPE_ERROR(
                em, "bpg_cli_proxy_rsp: save_append_info: %d: get meta fail(id=%u), ignore!",
                i, bpg_pkg_append_info_id(append_info));
            continue;
        }

        data = logic_require_data_get_or_create(require, meta, bpg_pkg_append_info_origin_size(append_info));
        if (data == NULL) {
            CPE_ERROR(
                em, "bpg_cli_proxy_rsp: save_append_info: %d: crate data with meta %s fail!"
                , i, dr_meta_name(meta));
            continue;
        }

        size = logic_data_capacity(data);
        if (bpg_pkg_get_append_data(pkg, append_info, logic_data_meta(data), logic_data_data(data), &size, em) != 0) {
            CPE_ERROR(
                em, "bpg_cli_proxy_rsp: save_append_info: %d: get append data fail!"
                , i);
            continue;
        }
    }

    return 0;
}

int bpg_cli_proxy_rsp(dp_req_t req, void * ctx, error_monitor_t em) {
    struct bpg_cli_proxy * proxy;
    bpg_pkg_t pkg;
    uint32_t sn;
    logic_require_t require;
 
    proxy = (struct bpg_cli_proxy *)ctx;

    pkg = bpg_pkg_from_dp_req(req);
    if (pkg == NULL) {
        CPE_ERROR(em, "bpg_cli_proxy_rsp: cast to pkg fail!");
        return -1;
    }

    sn = bpg_pkg_sn(pkg);
    require = logic_require_find(proxy->m_logic_mgr, sn);
    if (require == NULL) {
        CPE_ERROR(em, "bpg_cli_proxy_rsp: require not exist, sn=%u!", (unsigned int)sn);
        return -1;
    }

    if (bpg_cli_proxy_save_pkg_info(require, pkg, em) != 0
        || bpg_cli_proxy_save_main_body(require, pkg, em) != 0
        || bpg_cli_proxy_save_append_infos(require, pkg, em) != 0
        )
    {
        logic_require_set_error(require);
        return -1;
    }

    logic_require_set_done(require);
    return 0;
}
