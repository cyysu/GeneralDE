#include <assert.h> 
#include "cpe/aom/aom_obj_mgr.h"
#include "center_svr_ops.h"

center_cli_relation_t
center_cli_relation_create(center_cli_group_t provider, center_cli_group_t user) {
    center_svr_t svr = provider->m_svr;
    center_cli_relation_t relation;

    relation = mem_alloc(svr->m_alloc, sizeof(struct center_cli_relation));
    if (relation == NULL) {
        CPE_ERROR(svr->m_em, "%s: create relation: malloc fail!", center_svr_name(svr));
        return NULL;
    }

    relation->m_provider = provider;
    relation->m_user = user;

    TAILQ_INSERT_TAIL(&relation->m_provider->m_users, relation, m_next_for_provider);
    TAILQ_INSERT_TAIL(&relation->m_user->m_providers, relation, m_next_for_user);

    return relation;
}

void center_cli_relation_free(center_cli_relation_t relation) {
    center_svr_t svr = relation->m_provider->m_svr;

    TAILQ_REMOVE(&relation->m_provider->m_users, relation, m_next_for_provider);
    TAILQ_REMOVE(&relation->m_user->m_providers, relation, m_next_for_user);
    
    mem_free(svr->m_alloc, relation);
}
