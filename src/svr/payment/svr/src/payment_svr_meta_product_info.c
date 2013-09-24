#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/cfg/cfg_read.h" 
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_cfg.h"
#include "payment_svr_ops.h"

extern char g_metalib_svr_payment_pro[];

static int payment_svr_meta_product_info_cmp(void const * l, void const * r) {
    return strcmp(((PRODUCT_INFO const *)l)->product_id, ((PRODUCT_INFO const *)r)->product_id);
}

int payment_svr_meta_product_info_load(payment_svr_t svr, cfg_t cfg) {
    struct cfg_it child_it;
    uint16_t meta_count = cfg_seq_count(cfg);
    uint16_t i;
    PRODUCT_INFO * metas;
    LPDRMETA record_meta;

    if (meta_count == 0) {
        if (svr->m_product_infos) mem_free(svr->m_alloc, svr->m_product_infos);
        svr->m_product_infos = NULL;
        svr->m_product_info_count = 0;
        return 0;
    }

    record_meta = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_svr_payment_pro, "product_info");
    if (record_meta == NULL) {
        CPE_ERROR(svr->m_em, "%s: load bag meta: meta product_info not exist!", payment_svr_name(svr));
        return -1;
    }

    metas = mem_alloc(svr->m_alloc, sizeof(PRODUCT_INFO) * meta_count);
    cfg_it_init(&child_it, cfg);
    for(i = 0; i < meta_count; ++i) {
        cfg_t record_cfg = cfg_it_next(&child_it);

        assert(record_cfg);

        if (dr_cfg_read(&metas[i], sizeof(metas[i]), record_cfg, record_meta, 0, svr->m_em) <= 0) {
            CPE_ERROR(svr->m_em, "%s: load payment meta: load svr_payment_payment_meta fail!", payment_svr_name(svr));
            return -1;
        }
    }

    if (svr->m_product_infos) mem_free(svr->m_alloc, svr->m_product_infos);
    svr->m_product_infos = metas;
    svr->m_product_info_count = meta_count;

    qsort(metas, meta_count, sizeof(metas[0]), payment_svr_meta_product_info_cmp);
    return 0;
}

PRODUCT_INFO * payment_svr_meta_product_info_find(payment_svr_t svr, const char * product_id) {
    PRODUCT_INFO key;
    strncpy(key.product_id, product_id, sizeof(key.product_id));

    return bsearch(&key, svr->m_product_infos, svr->m_product_info_count, sizeof(key), payment_svr_meta_product_info_cmp);
}
