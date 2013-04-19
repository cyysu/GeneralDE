#include <assert.h> 
#include "cpe/aom/aom_obj_mgr.h"
#include "center_svr_ops.h"

center_cli_group_t
center_cli_group_get_or_create(center_svr_t svr, uint16_t svr_type) {
    struct center_cli_group key;
    center_cli_group_t group;

    key.m_svr_type = svr_type;
    group = cpe_hash_table_find(&svr->m_groups, &key);
    if (group == NULL) {
        group = mem_alloc(svr->m_alloc, sizeof(struct center_cli_group));
        if (group == NULL) {
            CPE_ERROR(
                svr->m_em, "%s: create group of svr type %d: malloc fail!",
                center_svr_name(svr), svr_type);
            return NULL;
        }

        group->m_svr = svr;
        group->m_svr_type = svr_type;
        group->m_svr_count = 0;
        TAILQ_INIT(&group->m_datas);

        cpe_hash_entry_init(&group->m_hh);
        cpe_hash_table_insert_unique(&svr->m_groups, group);
    }

    return group;
}

center_cli_group_t center_cli_group_find(center_svr_t svr, uint16_t svr_type) {
    struct center_cli_group key;
    key.m_svr_type = svr_type;
    return cpe_hash_table_find(&svr->m_groups, &key);
}

void center_cli_group_free(center_cli_group_t group) {
    center_svr_t svr = group->m_svr;

    assert(svr);
    assert(group->m_svr_count == 0);
    assert(TAILQ_EMPTY(&group->m_datas));

    cpe_hash_table_remove_by_ins(&svr->m_groups, group);

    mem_free(svr->m_alloc, group);
}

uint32_t center_cli_group_hash(center_cli_group_t group) {
    return group->m_svr_type;
}

int center_cli_group_eq(center_cli_group_t l, center_cli_group_t r) {
    return l->m_svr_type == r->m_svr_type;
}
