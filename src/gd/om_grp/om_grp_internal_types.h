#ifndef GD_OM_GRP_INTERNAL_TYPES_H
#define GD_OM_GRP_INTERNAL_TYPES_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/bitarry.h"
#include "cpe/dr/dr_types.h"
#include "gd/om_grp/om_grp_types.h"
#include "cpe/utils/range.h"

#define OM_GRP_OBJ_CONTROL_MAGIC (38438u)

TAILQ_HEAD(om_grp_entry_meta_list, om_grp_entry_meta);

struct om_grp_obj_mgr {
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    gd_om_mgr_t m_omm;
    om_grp_meta_t m_meta;
    LPDRMETALIB m_metalib;
    char * m_full_base;
    uint32_t m_full_capacity;
};

struct om_grp_meta {
    mem_allocrator_t m_alloc;
    const char * m_name;

    uint16_t m_omm_page_size;

    gd_om_class_id_t m_control_class_id;
    uint16_t m_control_obj_size;

    uint16_t m_page_count;
    uint16_t m_size_buf_start;
    uint16_t m_size_buf_count;

    struct om_grp_entry_meta_list m_entry_list;
    struct cpe_hash_table m_entry_ht;
};

struct om_grp_entry_data_normal {
    LPDRMETA m_data_meta;
};

struct om_grp_entry_data_list {
    LPDRMETA m_data_meta;
    uint32_t m_capacity;
    uint16_t m_size_idx;
};

struct om_grp_entry_data_ba {
    uint32_t m_capacity;
};

struct om_grp_entry_data_binary {
    uint32_t m_capacity;
};

union om_grp_entry_data {
    struct om_grp_entry_data_normal m_normal;
    struct om_grp_entry_data_list m_list;
    struct om_grp_entry_data_ba m_ba;
    struct om_grp_entry_data_binary m_binary;
};

struct om_grp_entry_meta {
    om_grp_meta_t m_meta;
    const char * m_name;
    om_grp_entry_type_t m_type;
    union om_grp_entry_data m_data;

    uint16_t m_page_begin;
    uint16_t m_page_count;
    gd_om_class_id_t m_class_id;
    uint16_t m_obj_size;
    uint16_t m_obj_align;

    TAILQ_ENTRY(om_grp_entry_meta) m_next;
    struct cpe_hash_entry m_hh;
};

extern cpe_hash_string_t om_grp_control_class_name;

#endif


