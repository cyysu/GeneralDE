#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/om/om_object.h"
#include "gd/om/om_manage.h"
#include "gd/om_grp/om_grp_meta.h"
#include "gd/om_grp/om_grp_obj.h"
#include "om_grp_internal_ops.h"

uint16_t om_grp_obj_list_count(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, const char * entry) {
    om_grp_entry_meta_t entry_meta = om_grp_entry_meta_find(mgr->m_meta, entry);
    if (entry_meta == NULL) {
        CPE_ERROR(mgr->m_em, "om_grp_obj_list_count: entry %s not exist!", entry);
        return 0;
    }

    return om_grp_obj_list_count_ex(mgr, obj, entry_meta);
}

void * om_grp_obj_list_at(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, const char * entry, uint16_t pos) {
    om_grp_entry_meta_t entry_meta = om_grp_entry_meta_find(mgr->m_meta, entry);
    if (entry_meta == NULL) {
        CPE_ERROR(mgr->m_em, "om_grp_obj_list_at: entry %s not exist!", entry);
        return NULL;
    }

    return om_grp_obj_list_at_ex(mgr, obj, entry_meta, pos);
}

int om_grp_obj_list_append(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, const char * entry, void * data) {
    om_grp_entry_meta_t entry_meta = om_grp_entry_meta_find(mgr->m_meta, entry);
    if (entry_meta == NULL) {
        CPE_ERROR(mgr->m_em, "om_grp_obj_list_append: entry %s not exist!", entry);
        return -1;
    }

    return om_grp_obj_list_append_ex(mgr, obj, entry_meta, data);
}

int om_grp_obj_list_insert(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, const char * entry, uint16_t pos, void * data) {
    om_grp_entry_meta_t entry_meta = om_grp_entry_meta_find(mgr->m_meta, entry);
    if (entry_meta == NULL) {
        CPE_ERROR(mgr->m_em, "om_grp_obj_list_insert: entry %s not exist!", entry);
        return -1;
    }

    return om_grp_obj_list_insert_ex(mgr, obj, entry_meta, pos, data);
}

int om_grp_obj_list_remove(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, const char * entry, uint16_t pos) {
    om_grp_entry_meta_t entry_meta = om_grp_entry_meta_find(mgr->m_meta, entry);
    if (entry_meta == NULL) {
        CPE_ERROR(mgr->m_em, "om_grp_obj_list_remove: entry %s not exist!", entry);
        return -1;
    }

    return om_grp_obj_list_remove_ex(mgr, obj, entry_meta, pos);
}

uint16_t * om_grp_obj_list_count_buf(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, om_grp_entry_meta_t entry) {
    return ((uint16_t *)(((char *)obj) + mgr->m_meta->m_size_buf_start)) + entry->m_data.m_list.m_size_idx;
}

uint16_t om_grp_obj_list_count_ex(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, om_grp_entry_meta_t entry) {
    assert(entry);
    assert(obj);
    assert(entry->m_type == om_grp_entry_type_list);
    return *om_grp_obj_list_count_buf(mgr, obj, entry);
}

void * om_grp_obj_list_at_ex(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, om_grp_entry_meta_t entry, uint16_t pos) {
    uint16_t element_size;
    uint16_t count_in_page;
    uint16_t page_pos;
    uint16_t pos_in_page;
    gd_om_oid_t oid;
    uint16_t * count;
    char * page_buf;

    assert(entry);
    assert(obj);
    assert(entry->m_type == om_grp_entry_type_list);

    if (pos >= entry->m_data.m_list.m_capacity) return NULL;

    count = om_grp_obj_list_count_buf(mgr, obj, entry);
    if (pos >= *count) return NULL;

    element_size = dr_meta_size(entry->m_data.m_list.m_data_meta);
    assert(element_size > 0);
    assert(entry->m_obj_size % element_size == 0);

    count_in_page = entry->m_obj_size / element_size;
    assert(count_in_page > 0);

    page_pos = pos / count_in_page;
    pos_in_page = pos % count_in_page;
    assert(page_pos < entry->m_page_count);

    oid = ((gd_om_oid_t *)obj)[entry->m_page_begin + page_pos];
    assert(oid != GD_OM_INVALID_OID);

    page_buf = ((char*)gd_om_obj_get(mgr->m_omm, oid, mgr->m_em));
    assert(page_buf);

    return page_buf + pos_in_page * element_size;
}

int om_grp_obj_list_append_ex(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, om_grp_entry_meta_t entry, void * data) {
    uint16_t next_pos;
    uint16_t element_size;
    uint16_t count_in_page;
    uint16_t page_pos;
    uint16_t pos_in_page;
    gd_om_oid_t * oid;
    uint16_t * count;
    char * page_buf;

    assert(entry);
    assert(obj);
    assert(entry->m_type == om_grp_entry_type_list);

    count = om_grp_obj_list_count_buf(mgr, obj, entry);
    if (*count >= entry->m_data.m_list.m_capacity) {
        CPE_ERROR(
            mgr->m_em, "om_grp_obj_list_append_ex: entry %s overflow, capacity=%d!",
            entry->m_name, entry->m_data.m_list.m_capacity);
        return -1;
    }

    next_pos = *count;

    element_size = dr_meta_size(entry->m_data.m_list.m_data_meta);
    assert(element_size > 0);
    assert(entry->m_obj_size % element_size == 0);

    count_in_page = entry->m_obj_size / element_size;
    assert(count_in_page > 0);

    page_pos = next_pos / count_in_page;
    pos_in_page = next_pos % count_in_page;

    assert(page_pos < entry->m_page_count);

    oid = ((gd_om_oid_t *)obj) + entry->m_page_begin + page_pos;
    if (*oid == GD_OM_INVALID_OID) {
        *oid = gd_om_obj_alloc(mgr->m_omm, om_grp_entry_meta_name_hs(entry), mgr->m_em);
        if (*oid == GD_OM_INVALID_OID) {
            CPE_ERROR(mgr->m_em, "om_grp_obj_list_append_ex: alloc %s buf fail!", entry->m_name);
            return -1;
        }
    }

    page_buf = ((char*)gd_om_obj_get(mgr->m_omm, *oid, mgr->m_em));
    if (page_buf == NULL) {
        CPE_ERROR(mgr->m_em, "om_grp_obj_list_append_ex: get buf fail!");
        return -1;
    }

    memcpy(page_buf + pos_in_page * element_size, data, element_size);
    (*count)++;
    return 0;
}

int om_grp_obj_list_insert_ex(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, om_grp_entry_meta_t entry, uint16_t pos, void * data) {
    return -1;
}

int om_grp_obj_list_remove_ex(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, om_grp_entry_meta_t entry, uint16_t pos) {
    return -1;
}
