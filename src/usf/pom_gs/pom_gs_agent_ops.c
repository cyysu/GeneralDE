#include <assert.h>
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/pom_grp/pom_grp_store.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "usf/logic/logic_require.h"
#include "usf/pom_gs/pom_gs_agent.h"
#include "pom_gs_internal_ops.h"

static int pom_gs_agent_build_data(
    char * buf,
    size_t capacity,
    pom_grp_store_entry_t entry)
{
    return 0;
}

int pom_gs_agent_obj_insert(pom_gs_agent_t agent, pom_grp_obj_mgr_t obj_mgr, pom_grp_obj_t obj, logic_require_t require) {
    struct pom_grp_store_table_it table_it;
    pom_grp_store_table_t table;

    if (agent->m_backend == NULL) {
        CPE_ERROR(
            agent->m_em, "%s: obj_insert: backend not exist!",
            pom_gs_agent_name(agent));
        goto ERROR;
    }

    if (agent->m_backend->insert == NULL) {
        CPE_ERROR(
            agent->m_em, "%s: obj_insert: backend %s not support insert!",
            pom_gs_agent_name(agent), agent->m_backend->name);
        goto ERROR;
    }

    while((table = pom_grp_store_table_it_next(&table_it))) {
        struct pom_grp_store_entry_it entry_it;
        pom_grp_store_entry_t entry;
        size_t capacity = dr_meta_size(pom_grp_store_table_meta(table));
        char * buf = pom_gs_agent_buf(agent, capacity);
        if (buf == NULL) {
            CPE_ERROR(
                agent->m_em, "%s: obj_insert: get buf for insert %s, capacity=%d",
                pom_gs_agent_name(agent), pom_grp_store_table_name(table), (int)capacity);
            goto ERROR;
        }
        bzero(buf, capacity);

        pom_grp_table_entries(table, &entry_it);

        while((entry = pom_grp_store_entry_it_next(&entry_it))) {
            if (pom_gs_agent_build_data(buf, capacity, entry) != 0) {
                CPE_ERROR(
                    agent->m_em, "%s: obj_insert: %s: bulid data for %s fail!",
                    pom_gs_agent_name(agent), pom_grp_store_table_name(table), pom_grp_store_entry_name(entry));
                goto ERROR;
            }
        }

        if (agent->m_backend->insert(
                table,
                buf,
                capacity,
                require,
                agent->m_backend_ctx) != 0)
        {
            CPE_ERROR(
                agent->m_em, "%s: obj_insert: backend %s insert fail!",
                pom_gs_agent_name(agent), agent->m_backend->name);
            goto ERROR;
        }
    }

    return 0;

ERROR:
    if (require) logic_require_set_error(require);
    return -1;
}

int pom_gs_agent_data_insert(
    pom_gs_agent_t agent,
    LPDRMETA meta, void * data, size_t capacity, 
    const char * entries,
    logic_require_t require)
{
    
    return 0;
}
