#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/om_grp/om_grp_meta.h"
#include "om_grp_internal_ops.h"
#include "om_grp_data.h"

om_grp_meta_t
om_grp_meta_create(
    mem_allocrator_t alloc,
    const char * name,
    uint16_t omm_page_size)
{
    char * buf;
    size_t name_len;
    om_grp_meta_t meta;

    name_len = strlen(name) + 1;

    buf = mem_alloc(alloc, CPE_PAL_ALIGN(name_len) + sizeof(struct om_grp_meta));
    if (buf == NULL) return NULL;

    memcpy(buf, name, name_len);

    meta = (om_grp_meta_t)(buf + CPE_PAL_ALIGN(name_len));
    meta->m_alloc = alloc;
    meta->m_name = buf;
    meta->m_omm_page_size = omm_page_size;
    meta->m_control_class_id = 1;
    meta->m_control_obj_size = 0;
    meta->m_page_count = 0;
    meta->m_size_buf_start = 0;
    meta->m_size_buf_count = 0;

    TAILQ_INIT(&meta->m_entry_list);

    if (cpe_hash_table_init(
            &meta->m_entry_ht,
            alloc,
            (cpe_hash_fun_t) om_grp_entry_meta_hash,
            (cpe_hash_cmp_t) om_grp_entry_meta_cmp,
            CPE_HASH_OBJ2ENTRY(om_grp_entry_meta, m_hh),
            -1) != 0)
    {
        mem_free(alloc, buf);
        return NULL;
    }

    return meta;
}

void om_grp_meta_free(om_grp_meta_t meta) {
    om_grp_entry_meta_free_all(meta);

    cpe_hash_table_fini(&meta->m_entry_ht);

    mem_free(meta->m_alloc, (void*)meta->m_name);
}

const char * om_grp_meta_name(om_grp_meta_t meta) {
    return meta->m_name;
}

om_grp_entry_meta_t
om_grp_entry_meta_find(om_grp_meta_t meta, const char * name) {
    struct om_grp_meta key;
    key.m_name = name;

    return (om_grp_entry_meta_t)cpe_hash_table_find(&meta->m_entry_ht, &key);
}

static om_grp_entry_meta_t om_grp_entry_meta_it_next(struct om_grp_entry_meta_it * it) {
    om_grp_entry_meta_t r;
    if (it->m_data == NULL) return NULL;

    r = it->m_data;
    it->m_data = TAILQ_NEXT((om_grp_entry_meta_t)it->m_data, m_next);
    return r;
}

void om_grp_entry_meta_it_init(om_grp_meta_t meta, om_grp_entry_meta_it_t it) {
    it->m_data = TAILQ_FIRST(&meta->m_entry_list);
    it->next = om_grp_entry_meta_it_next;
}

void om_grp_meta_dump(write_stream_t stream, om_grp_meta_t meta, int ident) {
    om_grp_entry_meta_t entry;

    stream_putc_count(stream, ' ', ident);
    stream_printf(stream, "om_grp_meta: name=%s", meta->m_name);

    stream_printf(
        stream, ", page-size=%d, class-id=%d, obj-size=%d, page-count=%d, size-buf-start=%d, size-buf-count=%d",
        meta->m_omm_page_size, (int)meta->m_control_class_id,
        meta->m_control_obj_size, meta->m_page_count, meta->m_size_buf_start, meta->m_size_buf_count);

    TAILQ_FOREACH(entry, &meta->m_entry_list, m_next) {
        stream_printf(stream, "\n");
        stream_putc_count(stream, ' ', ident ? (ident << 2) : 4);
        stream_printf(stream, "%s: ", entry->m_name);

        switch(entry->m_type) {
        case om_grp_entry_type_normal:
            stream_printf(
                stream, "entry-type=normal, data=type=%s",
                dr_meta_name(entry->m_data.m_normal.m_data_meta));
            break;
        case om_grp_entry_type_list:
            stream_printf(
                stream, "entry-type=list, data=type=%s, capacity=%d, size-idx=%d",
                dr_meta_name(entry->m_data.m_list.m_data_meta),
                (int)entry->m_data.m_list.m_capacity,
                (int)entry->m_data.m_list.m_size_idx);
            break;
        case om_grp_entry_type_ba:
            stream_printf(
                stream, "entry-type=ba, bit-capacity=%d",
                (int)entry->m_data.m_ba.m_bit_capacity);
            break;
        case om_grp_entry_type_binary:
            stream_printf(
                stream, "entry-type=binary, capacity=%d",
                (int)entry->m_data.m_binary.m_capacity);
            break;
        default:
            stream_printf(stream, "entry-type=unknown");
            break;
        }

        stream_printf(
            stream, ", page-begin=%d, page-count=%d, class-id=%d, obj-size=%d, obj-align=%d",
            entry->m_page_begin, entry->m_page_count, entry->m_class_id, entry->m_obj_size, entry->m_obj_align);
    }
}
