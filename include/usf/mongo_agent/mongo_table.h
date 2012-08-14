#ifndef USF_MONGO_TABLE_H
#define USF_MONGO_TABLE_H
#include "cpe/dr/dr_types.h"
#include "mongo_types.h"

#ifdef __cplusplus
extern "C" {
#endif

mongo_table_t
mongo_table_create(
    mongo_agent_t agent,
    const char * name,
    LPDRMETALIB metalib,
    size_t data_capacity);

void mongo_table_free(mongo_table_t table);

mongo_table_t
mongo_table_find(
    mongo_agent_t agent,
    const char * name);

const char * mongo_table_name(mongo_table_t table);
LPDRMETA mongo_table_meta(mongo_table_t table);
LPDRMETALIB mongo_table_metalib(mongo_table_t table);

#ifdef __cplusplus
}
#endif

#endif
