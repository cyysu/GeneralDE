#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/dr/dr_data.h"
#include "gd/app/app_log.h"
#include "usf/mongo_agent/mongo_agent.h"
#include "usf/mongo_agent/mongo_table.h"
#include "usf/mongo_agent/mongo_request.h"
#include "mongo_internal_ops.h"

mongo_request_t
mongo_request_get(
    mongo_agent_t agent,
    const char * table_name)
{
    struct mongo_request key;
    mongo_request_t request;

    key.m_name = table_name;
    request = (mongo_request_t)cpe_hash_table_find(&agent->m_requests, &key);
    if (request == NULL) {
        mongo_table_t table = mongo_table_find(agent, table_name);
        if (table == NULL) {
            CPE_ERROR(
                agent->m_em, "%s: request get: table %s not exist!",
                mongo_agent_name(agent), table_name);
            return NULL;
        }

        request = (mongo_request_t)mem_alloc(
            agent->m_alloc, sizeof(struct mongo_request) + table->m_data_capacity);
        if (request == NULL){
            CPE_ERROR(
                agent->m_em, "%s: request get: alloc request for %s fail, capacity==%d!",
                mongo_agent_name(agent), table_name, (int)table->m_data_capacity);
            return NULL;
        }

        request->m_agent = agent;
        request->m_name = table->m_name;
        request->m_table = table;

        cpe_hash_entry_init(&request->m_hh);
        if (cpe_hash_table_insert_unique(&agent->m_requests, request) != 0) {
            CPE_ERROR(
                agent->m_em, "%s: request get: insert request to agent for %s fail!",
                mongo_agent_name(agent), table_name);
            mem_free(agent->m_alloc, request);
            return NULL;
        }
    }

    assert(request);

    return request;
}

void mongo_request_free(mongo_request_t request) {
    cpe_hash_table_remove_by_ins(&request->m_agent->m_requests, request);

    mem_free(request->m_agent->m_alloc, request);
}

mongo_agent_t mongo_request_agent(mongo_request_t request) {
    return request->m_agent;
}

mongo_table_t
mongo_request_table(mongo_request_t request) {
    return request->m_table;
}

void * mongo_request_data_buf(mongo_request_t request) {
    return request + 1;
}

size_t mongo_request_data_capacity(mongo_request_t request) {
    return request->m_table->m_data_capacity;
}

uint32_t mongo_request_hash(const struct mongo_request * request) {
    return cpe_hash_str(request->m_name, strlen(request->m_name));
}

int mongo_request_cmp(const struct mongo_request * l, const struct mongo_request * r) {
    return strcmp(l->m_name, r->m_name) == 0;
}

void mongo_request_free_all(mongo_agent_t agent) {
    struct cpe_hash_it request_it;
    mongo_request_t request;

    cpe_hash_it_init(&request_it, &agent->m_requests);

    request = (mongo_request_t)cpe_hash_it_next(&request_it);
    while (request) {
        mongo_request_t next = (mongo_request_t)cpe_hash_it_next(&request_it);
        mongo_request_free(request);
        request = next;
    }
}

int mongo_request_add_record(mongo_request_t request, LPDRMETA meta, const void * data, size_t size) {
    if (meta != request->m_table->m_meta) {
        char * buf = (char *)mongo_request_data_buf(request);
        assert(buf);
        bzero(buf, request->m_table->m_data_capacity);
        dr_meta_copy_same_entry(
            buf, request->m_table->m_data_capacity, request->m_table->m_meta,
            data, size, meta, 0, request->m_agent->m_em);

        meta = request->m_table->m_meta;
        data = buf;
        size = request->m_table->m_data_capacity;
    }

    return 0;
}
