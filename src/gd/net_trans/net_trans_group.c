#include <assert.h>
#include "gd/net_trans/net_trans_manage.h"
#include "gd/net_trans/net_trans_group.h"
#include "gd/net_trans/net_trans_task.h"
#include "net_trans_internal_ops.h"

net_trans_group_t net_trans_group_create(net_trans_manage_t mgr, const char * name) {
    net_trans_group_t group;
    size_t name_len = strlen(name) + 1;

    group = mem_alloc(mgr->m_alloc, sizeof(struct net_trans_group) + name_len);
    if (group == NULL) {
        CPE_ERROR(mgr->m_em, "%s: group %s: create: alloc fail!", net_trans_manage_name(mgr), name);
        return NULL;
    }

    group->m_mgr = mgr;
    group->m_name = (void*)(group + 1);
    memcpy((void*)(group + 1), name, name_len);

    TAILQ_INIT(&group->m_tasks);

    cpe_hash_entry_init(&group->m_hh_for_mgr);
    if (cpe_hash_table_insert_unique(&mgr->m_groups, group) == 0) {
        CPE_ERROR(mgr->m_em, "%s: group %s: create: name duplicate!", net_trans_manage_name(mgr), name);
        mem_free(mgr->m_alloc, group);
        return NULL;
    }
    
    return group;
}

void net_trans_group_free(net_trans_group_t group) {
    net_trans_manage_t mgr = group->m_mgr;

    while(!TAILQ_EMPTY(&group->m_tasks)) {
        net_trans_task_free(TAILQ_FIRST(&group->m_tasks));
    }

    cpe_hash_table_remove_by_ins(&mgr->m_groups, group);

    mem_free(mgr->m_alloc, group);
}

void net_trans_group_free_all(net_trans_manage_t mgr) {
    struct cpe_hash_it group_it;
    net_trans_group_t group;

    cpe_hash_it_init(&group_it, &mgr->m_groups);

    group = cpe_hash_it_next(&group_it);
    while (group) {
        net_trans_group_t next = cpe_hash_it_next(&group_it);
        net_trans_group_free(group);
        group = next;
    }
}

uint32_t net_trans_group_hash(net_trans_group_t group) {
    return cpe_hash_str(group->m_name, strlen(group->m_name));
}

int net_trans_group_eq(net_trans_group_t l, net_trans_group_t r) {
    return strcmp(l->m_name, r->m_name) == 0;
}