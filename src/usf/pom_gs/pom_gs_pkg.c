#include <assert.h>
#include "cpe/dp/dp_request.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/pom_grp/pom_grp_store.h"
#include "gd/app/app_context.h"
#include "usf/pom_gs/pom_gs_agent.h"
#include "usf/pom_gs/pom_gs_pkg.h"
#include "pom_gs_internal_ops.h"

pom_gs_pkg_t
pom_gs_pkg_create(pom_gs_agent_t agent, size_t pkg_capacity) {
    dp_req_t dp_req;
    pom_gs_pkg_t pkg;
    uint32_t i;
    uint32_t entry_count = pom_grp_store_table_count(agent->m_pom_grp_store);
    size_t head_capacity = pom_gs_pkg_head_capacity(entry_count);
    struct pom_grp_store_table_it table_it;
    uint32_t control_start, control_end;

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

    pkg = (pom_gs_pkg_t)dp_req_data(dp_req);

    pkg->m_agent = agent;
    pkg->m_dp_req = dp_req;
    pkg->m_entry_count = entry_count;

    pom_grp_store_tables(agent->m_pom_grp_store, &table_it);

    control_start = sizeof(struct pom_gs_pkg) + entry_count * sizeof(struct pom_gs_pkg_data_entry);
    control_end = control_start;

    for(i = 0; i < entry_count; ++i) {
        struct pom_gs_pkg_data_entry * data_entry = &pkg->m_entries[i];
        LPDRMETA table_meta;
        uint32_t mask_capacity;

        data_entry->m_table = pom_grp_store_table_it_next(&table_it);
        assert(data_entry->m_table);

        table_meta = pom_grp_store_table_meta(data_entry->m_table);
        assert(table_meta);
        mask_capacity = cpe_ba_bytes_from_bits(dr_meta_entry_num(table_meta));

        data_entry->m_mask_start = control_end;
        data_entry->m_mask_capacity = control_end + mask_capacity;

        control_end += mask_capacity;

        data_entry->m_data_start = 0;
        data_entry->m_data_capacity = 0;
    }

    pkg->m_data_start = control_end;

    bzero(((char *)pkg) + control_start, control_end - control_start);

    dp_req_set_size(pkg->m_dp_req, pkg->m_data_start);

    return pkg;
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
        data_entry->m_data_start = 0;
        data_entry->m_data_capacity = 0;
    }

    dp_req_set_size(pkg->m_dp_req, pkg->m_data_start);
}

int pom_gs_pkg_buf_resize(pom_gs_pkg_t pkg, struct pom_gs_pkg_data_entry * data_entry, size_t capacity) {
    pom_gs_agent_t agent;
    int inc_size = (int)capacity - (int)data_entry->m_data_capacity;
    uint32_t next_index;

    agent = pkg->m_agent;

    if ((int)dp_req_size(pkg->m_dp_req) + inc_size > (int)dp_req_capacity(pkg->m_dp_req)) {
        CPE_ERROR(
            agent->m_em, "%s: pom_gs_pkg_buf_resize for %s: not enough buf, old-capacity=%d, new-capacity=%d, pkg-capacity=%d",
            pom_gs_agent_name(agent), pom_grp_store_table_name(data_entry->m_table),
            (int)data_entry->m_data_capacity, (int)capacity, (int)dp_req_capacity(pkg->m_dp_req));
        return -1;
    }

    next_index = data_entry - pkg->m_entries + 1;
    if (next_index + 1 < pkg->m_entry_count) {
        char * move_start = pom_gs_pkg_entry_buf(pkg, data_entry) + data_entry->m_data_capacity;
        int move_size = (int)dp_req_size(pkg->m_dp_req) - (int)data_entry->m_data_start - (int)data_entry->m_data_capacity;

        assert(move_size >= 0);
        memmove(move_start, move_start + inc_size, move_size);

        for(; next_index < pkg->m_entry_count; ++next_index) {
            pkg->m_entries[next_index].m_data_start += inc_size;
        }
    }

    if (inc_size > 0) {
        bzero(pom_gs_pkg_entry_buf(pkg, data_entry) + data_entry->m_data_capacity, inc_size);
    }

    data_entry->m_data_capacity = capacity;
    dp_req_set_size(pkg->m_dp_req, dp_req_size(pkg->m_dp_req) + inc_size);

    return 0;
}

void * pom_gs_pkg_table_buf(pom_gs_pkg_t pkg, const char * table_name, size_t * capacity) {
    struct pom_gs_pkg_data_entry * data_entry;
    uint32_t i;

    assert(capacity);

    for(i = 0; i < pkg->m_entry_count; ++i) {
        data_entry = &pkg->m_entries[i];
        if (strcmp(pom_grp_store_table_name(data_entry->m_table), table_name) == 0) break;
    }

    if (i >= pkg->m_entry_count) return NULL;

    LPDRMETA meta = pom_grp_store_table_meta(data_entry->m_table);
    if (*capacity == 0) *capacity = dr_meta_size(meta);

    if (pom_gs_pkg_buf_resize(pkg, data_entry, *capacity) != 0) {
        CPE_ERROR(
            pkg->m_agent->m_em, "%s: pom_gs_pkg_buf for %s: resize fail!",
            pom_gs_agent_name(pkg->m_agent), pom_grp_store_table_name(data_entry->m_table));
        return NULL;
    }

    return pom_gs_pkg_entry_buf(pkg, data_entry);
}

int pom_gs_pkg_table_buf_clear(pom_gs_pkg_t pkg, const char * table_name) {
    struct pom_gs_pkg_data_entry * data_entry;
    uint32_t i;

    for(i = 0; i < pkg->m_entry_count; ++i) {
        data_entry = &pkg->m_entries[i];
        if (strcmp(pom_grp_store_table_name(data_entry->m_table), table_name) == 0) break;
    }

    if (i >= pkg->m_entry_count) return -1;
    
    if (pom_gs_pkg_buf_resize(pkg, data_entry, 0) != 0) {
        CPE_ERROR(
            pkg->m_agent->m_em, "%s: pom_gs_pkg_clear for %s: resize fail!",
            pom_gs_agent_name(pkg->m_agent), pom_grp_store_table_name(data_entry->m_table));
        return -1;
    }

    return 0;
}

size_t pom_gs_pkg_table_buf_capacity(pom_gs_pkg_t pkg, const char * table_name) {
    struct pom_gs_pkg_data_entry * data_entry;
    uint32_t i;

    for(i = 0; i < pkg->m_entry_count; ++i) {
        data_entry = &pkg->m_entries[i];

        if (strcmp(pom_grp_store_table_name(data_entry->m_table), table_name) == 0) {
            return data_entry->m_data_capacity;
        }
    }

    return 0;
}

int pom_gs_pkg_set_entry(
    pom_gs_pkg_t pkg, const char * entry_name,
    LPDRMETA meta, void const * data, size_t data_size)
{
    pom_gs_agent_t agent = pkg->m_agent;
    pom_grp_store_entry_t store_entry = pom_grp_store_entry_find(agent->m_pom_grp_store, entry_name);
    (void)store_entry;

    
    return 0;
}

int pom_gs_pkg_set_entry_part(
    pom_gs_pkg_t pkg, const char * entry_name,
    LPDRMETA meta, void const * data, size_t data_size,
    const char * sub_entries)
{
    return 0;
}

CPE_HS_DEF_VAR(pom_gs_pkg_type_name, "pom_gs_pkg_type");
