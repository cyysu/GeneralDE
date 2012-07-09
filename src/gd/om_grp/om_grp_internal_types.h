#ifndef GD_OM_GRP_INTERNAL_TYPES_H
#define GD_OM_GRP_INTERNAL_TYPES_H
#include "cpe/dr/dr_types.h"
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/hash.h"
#include "gd/om_grp/om_grp_types.h"

TAILQ_HEAD(om_grp_entry_meta_list, om_grp_entry_meta);

struct om_grp_meta {
    mem_allocrator_t m_alloc;

    struct om_grp_entry_meta_list m_entry_list;
    struct cpe_hash_table m_entry_ht;
};

struct om_grp_entry_meta {
    om_grp_meta_t m_meta;
    const char * m_name;
    om_grp_entry_type_t m_type;
    LPDRMETA m_data_meta;
    uint32_t m_capacity;

    TAILQ_ENTRY(om_grp_entry_meta) m_next;
    struct cpe_hash_entry m_hh;
};

#endif


