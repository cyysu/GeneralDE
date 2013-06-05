#include <assert.h> 
#include "cpe/aom/aom_obj_mgr.h"
#include "center_svr_ops.h"

center_cli_group_t
center_cli_group_create(center_svr_t svr, const char * svr_type_name, uint16_t svr_type_id) {
    center_cli_group_t group;
    size_t name_len = strlen(svr_type_name) + 1;

    group = mem_alloc(svr->m_alloc, sizeof(struct center_cli_group) + name_len);
    if (group == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: create group of svr type %d: malloc fail!",
            center_svr_name(svr), svr_type_id);
        return NULL;
    }

    group->m_svr = svr;
    group->m_svr_type_name = (char *)(group + 1);
    group->m_svr_type = svr_type_id;
    group->m_svr_count = 0;
    TAILQ_INIT(&group->m_datas);
    TAILQ_INIT(&group->m_users);
    TAILQ_INIT(&group->m_providers);

    memcpy(group->m_svr_type_name, svr_type_name, name_len);

    cpe_hash_entry_init(&group->m_hh);
    cpe_hash_table_insert_unique(&svr->m_groups, group);

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

    while(!TAILQ_EMPTY(&group->m_providers)) {
        center_cli_relation_free(TAILQ_FIRST(&group->m_providers));
    }

    while(!TAILQ_EMPTY(&group->m_users)) {
        center_cli_relation_free(TAILQ_FIRST(&group->m_users));
    }

    cpe_hash_table_remove_by_ins(&svr->m_groups, group);

    mem_free(svr->m_alloc, group);
}

void center_cli_group_free_all(center_svr_t svr) {
    struct cpe_hash_it group_it;
    center_cli_group_t group;

    cpe_hash_it_init(&group_it, &svr->m_groups);

    group = cpe_hash_it_next(&group_it);
    while(group) {
        center_cli_group_t next = cpe_hash_it_next(&group_it);
        center_cli_group_free(group);
        group = next;
    }
}

center_cli_group_t center_cli_group_lsearch_by_name(center_svr_t svr, const char * svr_type_name) {
    struct cpe_hash_it group_it;
    center_cli_group_t group;

    cpe_hash_it_init(&group_it, &svr->m_groups);

    while((group = cpe_hash_it_next(&group_it))) {
        if (strcmp(group->m_svr_type_name, svr_type_name) == 0) return group;
    }

    return NULL;
}

uint32_t center_cli_group_hash(center_cli_group_t group) {
    return group->m_svr_type;
}

int center_cli_group_eq(center_cli_group_t l, center_cli_group_t r) {
    return l->m_svr_type == r->m_svr_type;
}
