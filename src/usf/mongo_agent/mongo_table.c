#include <assert.h>
#include "cpe/dr/dr_metalib_manage.h"
#include "usf/mongo_agent/mongo_table.h"
#include "usf/mongo_agent/mongo_agent.h"
#include "mongo_internal_ops.h"

mongo_table_t
mongo_table_create(
    mongo_agent_t agent,
    const char * name,
    LPDRMETALIB metalib,
    size_t data_capacity)
{
    mongo_table_t table;
    size_t name_len;
    LPDRMETA table_meta;

    table_meta = dr_lib_find_meta_by_name(metalib, name);
    if (table_meta == NULL) {
        CPE_ERROR(
            agent->m_em, "%s: mongo_table_crate meta %s not exist in metalib!",
            mongo_agent_name(agent), name);
        return NULL;
    }

    if (agent->m_result_metalib != NULL) {
        CPE_ERROR(
            agent->m_em, "%s: agent already have result metalib!",
            mongo_agent_name(agent));
        return NULL;
    }

    if (data_capacity == 0) data_capacity = dr_meta_size(table_meta);

    name_len = strlen(name) + 1;

    table = (mongo_table_t)mem_alloc(agent->m_alloc, sizeof(struct mongo_table) + name_len);
    if (table == NULL) return NULL;

    memcpy(table + 1, name, name_len);

    table->m_agent = agent;
    table->m_name = (const char *)(table + 1);
    table->m_meta = table_meta;
    table->m_metalib = metalib;
    table->m_data_capacity = data_capacity;
    table->m_result_meta = NULL;

    TAILQ_INSERT_TAIL(&agent->m_tables, table, m_next);

    return table;
}

void mongo_table_free(mongo_table_t table) {
    TAILQ_REMOVE(&table->m_agent->m_tables, table, m_next);
    mem_free(table->m_agent->m_alloc, table);
}

const char * mongo_table_name(mongo_table_t table) {
    return table->m_name;
}

LPDRMETALIB mongo_table_metalib(mongo_table_t table) {
    return table->m_metalib;
}

LPDRMETA mongo_table_meta(mongo_table_t table) {
    return table->m_meta;
}

mongo_table_t
mongo_table_find(mongo_agent_t agent, const char * name) {
    mongo_table_t table;

    TAILQ_FOREACH(table, &agent->m_tables, m_next) {
        if (strcmp(table->m_name, name) == 0) return table;
    }

    return NULL;
}
