#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_data.h"
#include "usf/logic/logic_manage.h"
#include "usf/logic/logic_require.h"
#include "usf/logic/logic_data.h"
#include "usf/logic_use/logic_uni_res.h"
#include "usf/logic_use/logic_data_dyn.h"
#include "protocol/logic_use/logic_uni_res_info.h"

extern char g_metalib_logic_use[];

int logic_uni_res_init(logic_require_t require, LPDRMETA meta, size_t record_capacity) {
    LPDRMETA res_info_meta;
    logic_data_t res_info_data;
    logic_data_t res_data;
    size_t res_data_capacity;
    error_monitor_t em;
    LOGIC_UNI_RES_INFO * res_info;

    em = logic_manage_em(logic_require_mgr(require));

    res_info_meta  = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_logic_use, "logic_uni_res_info");
    assert(res_info_meta);

    res_info_data = logic_require_data_get_or_create(require, res_info_meta, dr_meta_size(res_info_meta));
    if (res_info_data == NULL) {
        CPE_ERROR(em, "logic_uni_res_init: %s: create res_info_data fail!", logic_require_name(require));
        return -1;
    }
    res_info = (LOGIC_UNI_RES_INFO *)logic_data_data(res_info_data);
    assert(res_info);
    strncpy(res_info->res_name, dr_meta_name(meta), sizeof(res_info->res_name));

    res_data_capacity = logic_data_calc_capacity(meta, record_capacity, em);

    res_data = logic_require_data_get_or_create(require, meta, res_data_capacity);
    if (res_data == NULL) {
        CPE_ERROR(
            em, "logic_uni_res_init: %s: create res_data fail, capacity=%d!",
            logic_require_name(require), (int)res_data_capacity);
        return -1;
    }

    return 0;
}

void logic_uni_res_fini(logic_require_t require) {
    LPDRMETA res_info_meta;
    logic_data_t res_info_data;
    logic_data_t res_data;
    LOGIC_UNI_RES_INFO * res_info;

    res_info_meta  = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_logic_use, "logic_uni_res_info");
    assert(res_info_meta);

    res_info_data = logic_require_data_find(require, "logic_uni_res_info");
    if (res_info_data == NULL) return;

    res_info = (LOGIC_UNI_RES_INFO *)logic_data_data(res_info_data);
    assert(res_info);

    res_data = logic_require_data_find(require, res_info->res_name);
    if (res_data) logic_data_free(res_data);

    logic_data_free(res_info_data);
}

logic_data_t logic_uni_res_data(logic_require_t require) {
    LPDRMETA res_info_meta;
    logic_data_t res_info_data;
    LOGIC_UNI_RES_INFO * res_info;

    res_info_meta  = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_logic_use, "logic_uni_res_info");
    assert(res_info_meta);

    res_info_data = logic_require_data_find(require, "logic_uni_res_info");
    if (res_info_data == NULL) return NULL;

    res_info = (LOGIC_UNI_RES_INFO *)logic_data_data(res_info_data);
    assert(res_info);

    return logic_require_data_find(require, res_info->res_name);
}
