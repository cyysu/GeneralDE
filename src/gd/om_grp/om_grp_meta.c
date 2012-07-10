#include "cpe/pal/pal_platform.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/om_grp/om_grp_meta.h"
#include "om_grp_internal_ops.h"

om_grp_meta_t om_grp_meta_create(mem_allocrator_t alloc, const char * name) {
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

void om_grp_meta_dump(write_stream_t stream, om_grp_meta_t meta, int ident) {
    om_grp_entry_meta_t entry;

    stream_putc_count(stream, ' ', ident);
    stream_printf(stream, "om_grp_meta: name=%s", meta->m_name);

    TAILQ_FOREACH(entry, &meta->m_entry_list, m_next) {
        stream_printf(stream, "\n");
        stream_putc_count(stream, ' ', ident >> 2);
        stream_printf(stream, "%s:", entry->m_name);

        switch(entry->m_type) {
        case om_grp_entry_type_normal:
            stream_printf(
                stream, "entry-type=normal, data=type=%s",
                dr_meta_name(entry->m_data.m_normal.m_data_meta));
            break;
        case om_grp_entry_type_list:
            stream_printf(
                stream, "entry-type=list, data=type=%s, capacity=%d",
                dr_meta_name(entry->m_data.m_list.m_data_meta),
                (int)entry->m_data.m_list.m_capacity);
            break;
        case om_grp_entry_type_ba:
            stream_printf(
                stream, "entry-type=ba, capacity=%d",
                (int)entry->m_data.m_ba.m_capacity);
            break;
        case om_grp_entry_type_binary:
        default:
            stream_printf(stream, "entry-type=unknown");
            break;
        }
    }
}
