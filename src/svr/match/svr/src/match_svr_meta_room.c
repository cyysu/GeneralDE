#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/cfg/cfg_read.h" 
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_cfg.h"
#include "match_svr_ops.h"

extern char g_metalib_svr_match_pro[];

static int match_svr_meta_room_meta_cmp(void const * l, void const * r) {
    return ((int)((SVR_MATCH_ROOM_META const *)l)->match_room_type)
        - ((int)((SVR_MATCH_ROOM_META const *)r)->match_room_type);
}

int match_svr_meta_room_load(match_svr_t svr, cfg_t cfg) {
    struct cfg_it child_it;
    uint16_t meta_count;
    uint16_t i;
    SVR_MATCH_ROOM_META * metas;
    LPDRMETA record_meta;

    if (cfg == NULL) {
        CPE_INFO(svr->m_em, "%s: load room meta: no room meta!", match_svr_name(svr));
        svr->m_metas = NULL;
        svr->m_meta_count = 0;
        return 0;
    }

    meta_count = cfg_seq_count(cfg);

    if (meta_count == 0) {
        if (svr->m_metas) mem_free(svr->m_alloc, svr->m_metas);
        svr->m_metas = NULL;
        svr->m_meta_count = 0;
        return 0;
    }

    record_meta = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_svr_match_pro, "svr_match_room_meta");
    if (record_meta == NULL) {
        CPE_ERROR(svr->m_em, "%s: load room meta: meta svr_match_room_meta not exist!", match_svr_name(svr));
        return -1;
    }

    metas = mem_alloc(svr->m_alloc, sizeof(SVR_MATCH_ROOM_META) * meta_count);
    cfg_it_init(&child_it, cfg);
    for(i = 0; i < meta_count; ++i) {
        cfg_t record_cfg = cfg_it_next(&child_it);

        assert(record_cfg);

        if (dr_cfg_read(&metas[i], sizeof(metas[i]), record_cfg, record_meta, 0, svr->m_em) <= 0) {
            CPE_ERROR(svr->m_em, "%s: load room meta: load svr_match_room_meta fail!", match_svr_name(svr));
            return -1;
        }
    }

    if (svr->m_metas) mem_free(svr->m_alloc, svr->m_metas);
    svr->m_metas = metas;
    svr->m_meta_count = meta_count;

    qsort(metas, meta_count, sizeof(metas[0]), match_svr_meta_room_meta_cmp);
    return 0;
}

SVR_MATCH_ROOM_META * match_svr_meta_foom_find(match_svr_t svr, uint16_t match_room_type) {
    SVR_MATCH_ROOM_META key;
    key.match_room_type = match_room_type;

    return bsearch(&key, svr->m_metas, svr->m_meta_count, sizeof(key), match_svr_meta_room_meta_cmp);
}