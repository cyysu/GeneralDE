#include <assert.h>
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/pom_grp/pom_grp_store.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "usf/logic/logic_require.h"
#include "usf/pom_gs/pom_gs_agent.h"
#include "usf/pom_gs/pom_gs_pkg.h"
#include "pom_gs_internal_ops.h"

static void pom_gs_agent_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_pom_gs_agent = {
    "usf_pom_gs_agent",
    pom_gs_agent_clear
};

pom_gs_agent_t
pom_gs_agent_create(
    gd_app_context_t app,
    const char * name,
    pom_grp_meta_t pom_grp_meta,
    const char * main_entry,
    const char * key,
    mem_allocrator_t alloc,
    error_monitor_t em)
{
    pom_gs_agent_t agent;
    nm_node_t agent_node;

    agent_node = nm_instance_create(gd_app_nm_mgr(app), name, sizeof(struct pom_gs_agent));
    if (agent_node == NULL) return NULL;

    agent = (pom_gs_agent_t)nm_node_data(agent_node);
    agent->m_app = app;
    agent->m_alloc = alloc;
    agent->m_em = em;
    agent->m_pom_grp_meta = pom_grp_meta;
    agent->m_backend = NULL;
    agent->m_pkg_buf = NULL;
    agent->m_pkg_buf_size = 4 * 1024;
    agent->m_backend_ctx = NULL;

    agent->m_debug = 0;

    agent->m_pom_grp_store = pom_grp_store_create(alloc, pom_grp_meta, main_entry, key, em);
    if (agent->m_pom_grp_store == NULL) {
        CPE_ERROR(
            em, "%s: create pom_grp_store fail, main_entry=%s, key=%s",
            name, main_entry, key);
        nm_node_free(agent_node);
        return NULL;
    }

    nm_node_set_type(agent_node, &s_nm_node_type_pom_gs_agent);

    return agent;
}

static void pom_gs_agent_clear(nm_node_t node) {
    pom_gs_agent_t agent;
    agent = (pom_gs_agent_t)nm_node_data(node);

    if (agent->m_pkg_buf) {
        pom_gs_pkg_free(agent->m_pkg_buf);
        agent->m_pkg_buf = NULL;
    }

    pom_grp_store_free(agent->m_pom_grp_store);
}

void pom_gs_agent_free(pom_gs_agent_t agent) {
    nm_node_t agent_node;
    assert(agent);

    agent_node = nm_node_from_data(agent);
    if (nm_node_type(agent_node) != &s_nm_node_type_pom_gs_agent) return;
    nm_node_free(agent_node);
}

pom_gs_agent_t
pom_gs_agent_find(
    gd_app_context_t app,
    cpe_hash_string_t name)
{
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_pom_gs_agent) return NULL;
    return (pom_gs_agent_t)nm_node_data(node);
}

pom_gs_agent_t
pom_gs_agent_find_nc(
    gd_app_context_t app,
    const char * name)
{
    nm_node_t node;

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_pom_gs_agent) return NULL;
    return (pom_gs_agent_t)nm_node_data(node);
}

gd_app_context_t pom_gs_agent_app(pom_gs_agent_t agent) {
    return agent->m_app;
}

const char * pom_gs_agent_name(pom_gs_agent_t agent) {
    return nm_node_name(nm_node_from_data(agent));
}

cpe_hash_string_t
pom_gs_agent_name_hs(pom_gs_agent_t agent) {
    return nm_node_name_hs(nm_node_from_data(agent));
}

int pom_gs_agent_set_backend(pom_gs_agent_t agent, pom_gs_agent_backend_t backend, void * ctx) {
    if (agent->m_backend != NULL) {
        CPE_ERROR(
            agent->m_em, "%s: pom_gs_agent_set_backend: backend %s already exist, new backend %s!",
            pom_gs_agent_name(agent), agent->m_backend->name, backend->name);
        return -1;
    }

    agent->m_backend = backend;
    agent->m_backend_ctx = ctx;

    return 0;
}

int pom_gs_agent_remove_backend(pom_gs_agent_t agent) {
    if (agent->m_backend == NULL) {
        CPE_ERROR(
            agent->m_em, "%s: pom_gs_agent_remove_backend: backend not exist!",
            pom_gs_agent_name(agent));
        return -1;
    }

    agent->m_backend = NULL;
    agent->m_backend_ctx = NULL;
    return 0;
}

pom_gs_pkg_t pom_gs_agent_pkg_buf(pom_gs_agent_t agent) {
    if (agent->m_pkg_buf == NULL) {
        agent->m_pkg_buf = pom_gs_pkg_create(agent, agent->m_pkg_buf_size);
    }

    return agent->m_pkg_buf;
}
