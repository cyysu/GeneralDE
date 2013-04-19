#include <assert.h>
#include "svr/center/center_agent.h"
#include "center_agent_internal_ops.h"

center_agent_data_svr_t
center_agent_data_svr_create(center_agent_t agent, uint16_t svr_type, uint16_t svr_id) {
    center_agent_data_svr_t data;
    data = mem_alloc(agent->m_alloc, sizeof(struct center_agent_data_svr));
    if (data == NULL) {
        CPE_ERROR(agent->m_em, "%s: create data: malloc fail!", center_agent_name(agent));
        return NULL;
    }

    data->m_agent = agent;
    data->m_svr_type = svr_type;
    data->m_svr_id = svr_id;
    data->m_ip = 0;
    data->m_port = 0;

    cpe_hash_entry_init(&data->m_hh);
    if (cpe_hash_table_insert_unique(&agent->m_svrs, data) != 0) {
        CPE_ERROR(
            agent->m_em, "%s: create data: insert fail, %d.%d already exist!",
            center_agent_name(agent), svr_type, svr_id);
        mem_free(agent->m_alloc, data);
        return NULL;
    }
    
    data->m_group = center_agent_data_group_find(agent, svr_type);
    if (data->m_group == NULL) {
        CPE_ERROR(
            agent->m_em, "%s: create data: get_or_create group of %d fail!",
            center_agent_name(agent), svr_type);
        cpe_hash_table_remove_by_ins(&agent->m_svrs, data);
        mem_free(agent->m_alloc, data);
        return NULL;
    }

    TAILQ_INSERT_TAIL(&data->m_group->m_svrs, data, m_next);

    return data;
}

void center_agent_data_svr_free(center_agent_data_svr_t svr) {
    center_agent_t agent = svr->m_agent;

    assert(agent);

    /*remove from group*/
    assert(svr->m_group);
    TAILQ_REMOVE(&svr->m_group->m_svrs, svr, m_next);
    svr->m_group = NULL;

    /*remove from svr*/
    cpe_hash_table_remove_by_ins(&agent->m_svrs, svr);

    mem_free(agent->m_alloc, svr);
}

void center_agent_data_svr_free_all(center_agent_t agent) {
    struct cpe_hash_it data_it;
    center_agent_data_svr_t data;

    cpe_hash_it_init(&data_it, &agent->m_svrs);

    data = cpe_hash_it_next(&data_it);
    while(data) {
        center_agent_data_svr_t next = cpe_hash_it_next(&data_it);
        center_agent_data_svr_free(data);
        data = next;
    }
}

center_agent_data_svr_t
center_agent_data_svr_find(center_agent_t agent, uint16_t svr_type, uint16_t svr_id) {
    struct center_agent_data_svr key;

    key.m_svr_type = svr_type;
    key.m_svr_id = svr_id;

    return cpe_hash_table_find(&agent->m_svrs, &key);
}

void center_agent_data_svr_sync(center_agent_t agent, SVR_CENTER_SVR_INFO const * info) {
    center_agent_data_svr_t svr = center_agent_data_svr_find(agent, info->id.svr_type, info->id.svr_id);
    if (svr == NULL) {
        svr = center_agent_data_svr_create(agent, info->id.svr_type, info->id.svr_id);
        if (svr == NULL) {
            CPE_ERROR(
                agent->m_em, "%s: synch svr info %d.%d: create svr fail!",
                center_agent_name(agent), info->id.svr_type, info->id.svr_id);
            return;
        }
    }

    if (info->ip == svr->m_ip || info->port == svr->m_port) return;

    svr->m_ip = info->ip;
    svr->m_port = info->port;
}

uint32_t center_agent_data_svr_hash(center_agent_data_svr_t svr) {
    return ((uint32_t)svr->m_svr_type) << 16 | svr->m_svr_id;
}

int center_agent_data_svr_eq(center_agent_data_svr_t l, center_agent_data_svr_t r) {
    return l->m_svr_type == r->m_svr_type && l->m_svr_id == r->m_svr_id;
}
