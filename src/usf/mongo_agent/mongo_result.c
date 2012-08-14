#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/dr/dr_data.h"
#include "gd/app/app_log.h"
#include "usf/logic/logic_require.h"
#include "usf/mongo_agent/mongo_agent.h"
#include "usf/mongo_agent/mongo_table.h"
#include "usf/mongo_agent/mongo_result.h"
#include "mongo_internal_ops.h"

mongo_result_t
mongo_result_get(
    mongo_agent_t agent,
    logic_require_t require,
    const char * table_name)
{
    struct mongo_result key;
    mongo_result_t result;

    key.m_name = table_name;
    result = (mongo_result_t)cpe_hash_table_find(&agent->m_results, &key);
    if (result == NULL) {
        mongo_table_t table = mongo_table_find(agent, table_name);
        if (table == NULL) {
            CPE_ERROR(
                agent->m_em, "%s: result get: table %s not exist!",
                mongo_agent_name(agent), table_name);
            return NULL;
        }

        result = (mongo_result_t)mem_alloc(
            agent->m_alloc, sizeof(struct mongo_result) + table->m_data_capacity);
        if (result == NULL){
            CPE_ERROR(
                agent->m_em, "%s: result get: alloc result for %s fail, capacity==%d!",
                mongo_agent_name(agent), table_name, (int)table->m_data_capacity);
            return NULL;
        }

        result->m_agent = agent;
        result->m_name = table->m_name;
        result->m_table = table;
        result->m_data = NULL;

        cpe_hash_entry_init(&result->m_hh);
        if (cpe_hash_table_insert_unique(&agent->m_results, result) != 0) {
            CPE_ERROR(
                agent->m_em, "%s: result get: insert result to agent for %s fail!",
                mongo_agent_name(agent), table_name);
            mem_free(agent->m_alloc, result);
            return NULL;
        }
    }

    assert(result);

    return result;
}

void mongo_result_free(mongo_result_t result) {
    cpe_hash_table_remove_by_ins(&result->m_agent->m_results, result);

    mem_free(result->m_agent->m_alloc, result);
}

mongo_agent_t mongo_result_agent(mongo_result_t result) {
    return result->m_agent;
}

mongo_table_t
mongo_result_table(mongo_result_t result) {
    return result->m_table;
}

uint32_t mongo_result_hash(const struct mongo_result * result) {
    return cpe_hash_str(result->m_name, strlen(result->m_name));
}

int mongo_result_cmp(const struct mongo_result * l, const struct mongo_result * r) {
    return strcmp(l->m_name, r->m_name) == 0;
}

void mongo_result_free_all(mongo_agent_t agent) {
    struct cpe_hash_it result_it;
    mongo_result_t result;

    cpe_hash_it_init(&result_it, &agent->m_results);

    result = (mongo_result_t)cpe_hash_it_next(&result_it);
    while (result) {
        mongo_result_t next = (mongo_result_t)cpe_hash_it_next(&result_it);
        mongo_result_free(result);
        result = next;
    }
}
