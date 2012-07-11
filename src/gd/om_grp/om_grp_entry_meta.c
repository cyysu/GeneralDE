#include "cpe/utils/bitarry.h"
#include "cpe/pal/pal_platform.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/om_grp/om_grp_meta.h"
#include "om_grp_internal_ops.h"

static om_grp_entry_meta_t
om_grp_entry_meta_create_i(
    om_grp_meta_t meta,
    const char * entry_name,
    om_grp_entry_type_t type,
    uint16_t obj_size,
    uint16_t obj_align,
    error_monitor_t em)
{
    char * buf;
    size_t name_len;
    om_grp_entry_meta_t entry_meta;
    om_grp_entry_meta_t pre_entry_meta;

    name_len = strlen(entry_name) + 1;

    buf = mem_alloc(meta->m_alloc, CPE_PAL_ALIGN(name_len) + sizeof(struct om_grp_entry_meta));
    if (buf == NULL) return NULL;

    memcpy(buf, entry_name, name_len);

    pre_entry_meta = TAILQ_FIRST(&meta->m_entry_list);

    entry_meta = (om_grp_entry_meta_t)(buf + CPE_PAL_ALIGN(name_len));
    entry_meta->m_meta = meta;
    entry_meta->m_name = buf;
    entry_meta->m_type = type;
    entry_meta->m_obj_size = obj_size;
    entry_meta->m_obj_align = obj_align;

    entry_meta->m_page_begin = 
        pre_entry_meta
        ? pre_entry_meta->m_page_begin + pre_entry_meta->m_page_count
        : 0;

    entry_meta->m_page_count = meta->m_omm_page_size / obj_size;

    if (entry_meta->m_page_count < 0) {
        
    }

    entry_meta->m_class_id = 
        pre_entry_meta
        ? pre_entry_meta->m_class_id + 1
        : meta->m_omm_control_class_id + 1;

    cpe_hash_entry_init(&entry_meta->m_hh);
    if (cpe_hash_table_insert_unique(&meta->m_entry_ht, entry_meta) != 0) {
        mem_free(meta->m_alloc, buf);
        return NULL;
    }

    TAILQ_INSERT_TAIL(&meta->m_entry_list, entry_meta, m_next);

    return entry_meta;
}

om_grp_entry_meta_t
om_grp_entry_meta_normal_create(
    om_grp_meta_t meta,
    const char * entry_name,
    LPDRMETA entry_meta,
    error_monitor_t em)
{
    om_grp_entry_meta_t r =
        om_grp_entry_meta_create_i(
            meta,
            entry_name,
            om_grp_entry_type_normal,
            dr_meta_size(entry_meta),
            dr_meta_align(entry_meta), em);
    if (r) r->m_data.m_normal.m_data_meta = entry_meta;
    return r;
}

om_grp_entry_meta_t
om_grp_entry_meta_list_create(
    om_grp_meta_t meta,
    const char * entry_name,
    LPDRMETA entry_meta, uint32_t count_per_page, uint32_t capacity,
    error_monitor_t em)
{
    om_grp_entry_meta_t r =
        om_grp_entry_meta_create_i(
            meta, entry_name, om_grp_entry_type_list,
            count_per_page * dr_meta_size(entry_meta), dr_meta_align(entry_meta),
            em);

    if (r) {
        r->m_data.m_list.m_data_meta = entry_meta;
        r->m_data.m_list.m_capacity = capacity;
    }

    return r;
}

om_grp_entry_meta_t
om_grp_entry_meta_ba_create(
    om_grp_meta_t meta,
    const char * entry_name,
    uint32_t capacity,
    error_monitor_t em)
{
    om_grp_entry_meta_t r =
        om_grp_entry_meta_create_i(
            meta, entry_name, om_grp_entry_type_ba,
            cpe_ba_bytes_from_bits(capacity), 1, em);

    if (r) r->m_data.m_ba.m_capacity = capacity;

    return r;
}

om_grp_entry_meta_t
om_grp_entry_meta_binary_create(
    om_grp_meta_t meta,
    const char * entry_name,
    uint32_t capacity,
    error_monitor_t em)
{
    om_grp_entry_meta_t r =
        om_grp_entry_meta_create_i(
            meta, entry_name, om_grp_entry_type_binary, capacity, 1, em);

    if (r) r->m_data.m_ba.m_capacity = capacity;

    return r;
}

void om_grp_entry_meta_free(om_grp_entry_meta_t entry_meta) {
    om_grp_meta_t meta = entry_meta->m_meta;

    TAILQ_REMOVE(&meta->m_entry_list, entry_meta, m_next);
    cpe_hash_table_remove_by_ins(&meta->m_entry_ht, meta);

    mem_free(meta->m_alloc, (void*)entry_meta->m_name);
}

const char * om_grp_entry_meta_name(om_grp_entry_meta_t entry_meta) {
    return entry_meta->m_name;
}

om_grp_entry_type_t om_grp_entry_meta_type(om_grp_entry_meta_t entry_meta) {
    return entry_meta->m_type;
}

uint32_t om_grp_entry_meta_hash(const struct om_grp_entry_meta * entry_meta) {
    return cpe_hash_str(entry_meta->m_name, strlen(entry_meta->m_name));
}

int om_grp_entry_meta_cmp(const struct om_grp_entry_meta * l, const struct om_grp_entry_meta * r) {
    return strcmp(l->m_name, r->m_name) == 0;
}

void om_grp_entry_meta_free_all(om_grp_meta_t meta) {
    struct cpe_hash_it entry_meta_it;
    om_grp_entry_meta_t entry_meta;

    cpe_hash_it_init(&entry_meta_it, &meta->m_entry_ht);

    entry_meta = cpe_hash_it_next(&entry_meta_it);
    while (entry_meta) {
        om_grp_entry_meta_t next = cpe_hash_it_next(&entry_meta_it);
        om_grp_entry_meta_free(entry_meta);
        entry_meta = next;
    }
}
