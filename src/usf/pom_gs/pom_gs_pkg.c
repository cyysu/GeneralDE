#include <assert.h>
#include "cpe/dp/dp_request.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/pom_grp/pom_grp_store.h"
#include "gd/app/app_context.h"
#include "usf/pom_gs/pom_gs_agent.h"
#include "usf/pom_gs/pom_gs_pkg.h"
#include "pom_gs_internal_ops.h"

pom_gs_pkg_t
pom_gs_pkg_create(
    pom_gs_agent_t agent,
    size_t pkg_capacity)
{
    dp_req_t dp_req;
    pom_gs_pkg_t pom_gs_pkg;
    uint32_t i;
    uint32_t entry_count = pom_grp_store_table_count(agent->m_pom_grp_store);
    size_t head_capacity = pom_gs_pkg_head_capacity(entry_count);
    struct pom_grp_store_table_it table_it;

    if (pkg_capacity < head_capacity) {
        CPE_ERROR(
            agent->m_em, "%s: pom_gs_pkg_create: pkg_capacity too small, entry-count=%d, head-capacity=%d, pkg-capacity=%d",
            pom_gs_agent_name(agent), (int)entry_count, (int)head_capacity, (int)pkg_capacity);
        return NULL;
    }

    dp_req = dp_req_create(
        gd_app_dp_mgr(agent->m_app),
        pom_gs_pkg_type_name,
        sizeof(struct pom_gs_pkg) + pkg_capacity);
    if (dp_req == NULL) return NULL;

    pom_gs_pkg = (pom_gs_pkg_t)dp_req_data(dp_req);

    pom_gs_pkg->m_agent = agent;
    pom_gs_pkg->m_dp_req = dp_req;
    pom_gs_pkg->m_entry_count = entry_count;

    pom_grp_store_tables(agent->m_pom_grp_store, &table_it);

    for(i = 0; i < entry_count; ++i) {
        struct pom_gs_pkg_data_entry * data_entry = &pom_gs_pkg->m_entries[i];

        data_entry->m_table = pom_grp_store_table_it_next(&table_it);
        data_entry->m_start = 0;
        data_entry->m_capacity = 0;
    }

    return pom_gs_pkg;
}

void pom_gs_pkg_free(pom_gs_pkg_t req) {
    dp_req_free(req->m_dp_req);
}

pom_gs_agent_t pom_gs_pkg_agent(pom_gs_pkg_t req) {
    return req->m_agent;
}

void pom_gs_pkg_init(pom_gs_pkg_t pkg) {
    uint32_t i;

    for(i = 0; i < pkg->m_entry_count; ++i) {
        struct pom_gs_pkg_data_entry * data_entry = &pkg->m_entries[i];
        data_entry->m_start = 0;
        data_entry->m_capacity = 0;
    }

    dp_req_set_size(pkg->m_dp_req, pom_gs_pkg_head_capacity(pkg->m_entry_count));
}

static int pom_gs_pkg_buf_resize(pom_gs_pkg_t pkg, struct pom_gs_pkg_data_entry * data_entry, size_t capacity) {
    pom_gs_agent_t agent;
    size_t inc_size = capacity - data_entry->m_capacity;
    uint32_t index;

    agent = pkg->m_agent;

    if (dp_req_size(pkg->m_dp_req) + inc_size > dp_req_capacity(pkg->m_dp_req)) {
        CPE_ERROR(
            agent->m_em, "%s: pom_gs_pkg_buf_resize for %s: not enough buf, old-capacity=%d, new-capacity=%d, pkg-capacity=%d",
            pom_gs_agent_name(agent), pom_grp_store_table_name(data_entry->m_table),
            (int)data_entry->m_capacity, (int)capacity, (int)dp_req_capacity(pkg->m_dp_req));
        return -1;
    }

    index = data_entry - pkg->m_entries;
    if (index != pkg->m_entry_count - 1) {
        char * move_start = pom_gs_pkg_entry_buf(pkg, data_entry) + data_entry->m_capacity;
        int move_size = (int)dp_req_size(pkg->m_dp_req) - (int)data_entry->m_start - (int)data_entry->m_capacity;

        assert(move_size >= 0);

        memmove(move_start, move_start + inc_size, move_size);
    }

    data_entry->m_capacity = capacity;
    dp_req_set_size(pkg->m_dp_req, dp_req_size(pkg->m_dp_req) + inc_size);

    return 0;
}

void * pom_gs_pkg_buf(pom_gs_pkg_t pkg, const char * table_name, size_t capacity) {
    struct pom_gs_pkg_data_entry * data_entry;
    uint32_t i;

    for(i = 0; i < pkg->m_entry_count; ++i) {
        data_entry = &pkg->m_entries[i];

        if (strcmp(pom_grp_store_table_name(data_entry->m_table), table_name) == 0) break;;
    }

    if (i >= pkg->m_entry_count) return NULL;

    if (capacity == 0) capacity = dr_meta_size(pom_grp_store_table_meta(data_entry->m_table));

    if (pom_gs_pkg_buf_resize(pkg, data_entry, capacity) != 0) {
        CPE_ERROR(
            pkg->m_agent->m_em, "%s: pom_gs_pkg_buf for %s: resize fail!",
            pom_gs_agent_name(pkg->m_agent), pom_grp_store_table_name(data_entry->m_table));
        return NULL;
    }

    return pom_gs_pkg_entry_buf(pkg, data_entry);
}

size_t pom_gs_pkg_buf_capacity(pom_gs_pkg_t pkg, const char * table_name) {
    struct pom_gs_pkg_data_entry * data_entry;
    uint32_t i;

    for(i = 0; i < pkg->m_entry_count; ++i) {
        data_entry = &pkg->m_entries[i];

        if (strcmp(pom_grp_store_table_name(data_entry->m_table), table_name) == 0) {
            return data_entry->m_capacity;
        }
    }

    return 0;
}
