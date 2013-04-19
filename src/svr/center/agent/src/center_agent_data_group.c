#include <assert.h> 
#include "svr/center/center_agent.h"
#include "center_agent_internal_ops.h"

center_agent_data_group_t
center_agent_data_group_create(center_agent_t agent, uint16_t svr_type) {
    center_agent_data_group_t group;

    group = mem_alloc(agent->m_alloc, sizeof(struct center_agent_data_group));
    if (group == NULL) {
        CPE_ERROR(
            agent->m_em, "%s: create group of svr type %d: malloc fail!",
            center_agent_name(agent), svr_type);
        return NULL;
    }

    group->m_agent = agent;
    group->m_svr_type = svr_type;
    TAILQ_INIT(&group->m_svrs);

    cpe_hash_entry_init(&group->m_hh);
    if (cpe_hash_table_insert_unique(&agent->m_groups, group) != 0) {
        CPE_ERROR(
            agent->m_em, "%s: create group of svr type %d: insert fail!",
            center_agent_name(agent), svr_type);
        mem_free(agent->m_alloc, group);
        return NULL;
    }

    return group;
}

void center_agent_data_group_free(center_agent_data_group_t group) {
    center_agent_t agent = group->m_agent;

    assert(agent);

    while(!TAILQ_EMPTY(&group->m_svrs)) {
        center_agent_data_svr_free(TAILQ_FIRST(&group->m_svrs));
    }

    cpe_hash_table_remove_by_ins(&agent->m_groups, group);

    mem_free(agent->m_alloc, group);
}

void center_agent_data_group_free_all(center_agent_t agent) {
    struct cpe_hash_it group_it;
    center_agent_data_group_t group;

    cpe_hash_it_init(&group_it, &agent->m_groups);

    group = cpe_hash_it_next(&group_it);
    while(group) {
        center_agent_data_group_t next = cpe_hash_it_next(&group_it);
        center_agent_data_group_free(group);
        group = next;
    }
}

center_agent_data_group_t
center_agent_data_group_find(center_agent_t agent, uint16_t svr_type) {
    struct center_agent_data_group key;

    key.m_svr_type = svr_type;
    return cpe_hash_table_find(&agent->m_groups, &key);
}

uint32_t center_agent_data_group_hash(center_agent_data_group_t group) {
    return group->m_svr_type;
}

int center_agent_data_group_eq(center_agent_data_group_t l, center_agent_data_group_t r) {
    return l->m_svr_type == r->m_svr_type;
}
