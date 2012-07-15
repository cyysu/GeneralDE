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

int om_grp_obj_list_sort(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, const char * entry, int (*cmp)(void const *, void const *)) {
    om_grp_entry_meta_t entry_meta = om_grp_entry_meta_find(mgr->m_meta, entry);
    if (entry_meta == NULL) {
        CPE_ERROR(mgr->m_em, "om_grp_obj_list_sort: entry %s not exist!", entry);
        return -1;
    }

    return om_grp_obj_list_sort_ex(mgr, obj, entry_meta, cmp);
}

void * om_grp_obj_list_bsearch(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, const char * entry, void const * key, int (*cmp)(void const *, void const *)) {
    om_grp_entry_meta_t entry_meta = om_grp_entry_meta_find(mgr->m_meta, entry);
    if (entry_meta == NULL) {
        CPE_ERROR(mgr->m_em, "om_grp_obj_list_bsearch: entry %s not exist!", entry);
        return NULL;
    }

    return om_grp_obj_list_bsearch_ex(mgr, obj, entry_meta, key, cmp);
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
    assert(page_buf);

    memcpy(page_buf + pos_in_page * element_size, data, element_size);
    (*count)++;
    return 0;
}

int om_grp_obj_list_insert_ex(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, om_grp_entry_meta_t entry, uint16_t pos, void * data) {
    uint16_t element_size;
    uint16_t count_in_page;
    uint16_t insert_page_pos;
    uint16_t insert_pos_in_page;
    uint16_t last_page_pos;
    uint16_t last_pos_in_page;
    gd_om_oid_t * oid;
    uint16_t * count;
    char * page_buf;

    assert(entry);
    assert(obj);
    assert(entry->m_type == om_grp_entry_type_list);

    count = om_grp_obj_list_count_buf(mgr, obj, entry);
    if (*count == pos) return om_grp_obj_list_append_ex(mgr, obj, entry, data);

    if (pos > *count) {
        CPE_ERROR(
            mgr->m_em, "om_grp_obj_list_append_ex: entry %s: insert pos invalid, count=%d!",
            entry->m_name, *count);
        return -1;
    }

    if (*count >= entry->m_data.m_list.m_capacity) {
        CPE_ERROR(
            mgr->m_em, "om_grp_obj_list_insert_ex: entry %s: insert overflow, capacity=%d!",
            entry->m_name, entry->m_data.m_list.m_capacity);
        return -1;
    }

    element_size = dr_meta_size(entry->m_data.m_list.m_data_meta);
    assert(element_size > 0);
    assert(entry->m_obj_size % element_size == 0);

    count_in_page = entry->m_obj_size / element_size;
    assert(count_in_page > 0);

    insert_page_pos = pos / count_in_page;
    insert_pos_in_page = pos % count_in_page;
    assert(insert_page_pos < entry->m_page_count);

    last_page_pos = (*count) / count_in_page;
    last_pos_in_page = (*count) % count_in_page;
    assert(last_page_pos < entry->m_page_count);

    oid = ((gd_om_oid_t *)obj) + entry->m_page_begin + last_page_pos;
    if (*oid == GD_OM_INVALID_OID) {
        *oid = gd_om_obj_alloc(mgr->m_omm, om_grp_entry_meta_name_hs(entry), mgr->m_em);
        if (*oid == GD_OM_INVALID_OID) {
            CPE_ERROR(mgr->m_em, "om_grp_obj_list_insert_ex: alloc %s buf fail!", entry->m_name);
            return -1;
        }
    }

    page_buf = ((char*)gd_om_obj_get(mgr->m_omm, *oid, mgr->m_em));
    assert(page_buf);

    while(last_page_pos > insert_page_pos) {
        char * pre_page_buf;

        if (last_pos_in_page > 0) {
            memmove(page_buf + element_size, page_buf, element_size * (last_pos_in_page - 1));
        }

        --oid;
        assert(*oid != GD_OM_INVALID_OID);

        pre_page_buf = ((char*)gd_om_obj_get(mgr->m_omm, *oid, mgr->m_em));
        assert(pre_page_buf);

        memmove(page_buf, pre_page_buf + element_size * (count_in_page - 1), element_size);

        page_buf = pre_page_buf;
        --last_page_pos;
        last_pos_in_page = count_in_page - 1;
    }

    
    assert(*oid != GD_OM_INVALID_OID);
    assert(page_buf);

    assert(*oid == *(((gd_om_oid_t *)obj) + entry->m_page_begin + insert_page_pos));
    assert(page_buf == gd_om_obj_get(mgr->m_omm, *oid, mgr->m_em));

    if (insert_pos_in_page + 1 < count_in_page) {
        memmove(
            page_buf + element_size * (insert_pos_in_page + 1),
            page_buf + element_size * insert_pos_in_page,
            element_size * (count_in_page - (insert_pos_in_page + 1)));
    }

    memcpy(page_buf + element_size * insert_pos_in_page, data, element_size);
    ++(*count);
    return 0;
}

int om_grp_obj_list_remove_ex(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, om_grp_entry_meta_t entry, uint16_t pos) {
    uint16_t element_size;
    uint16_t count_in_page;
    uint16_t remove_page_pos;
    uint16_t remove_pos_in_page;
    uint16_t last_page_pos;
    uint16_t last_pos_in_page;
    gd_om_oid_t * oid;
    uint16_t * count;
    char * page_buf;
    uint16_t last_page_left_count;

    assert(entry);
    assert(obj);
    assert(entry->m_type == om_grp_entry_type_list);

    count = om_grp_obj_list_count_buf(mgr, obj, entry);
    if (pos >= *count) return -1;

    element_size = dr_meta_size(entry->m_data.m_list.m_data_meta);
    assert(element_size > 0);
    assert(entry->m_obj_size % element_size == 0);

    count_in_page = entry->m_obj_size / element_size;
    assert(count_in_page > 0);

    remove_page_pos = pos / count_in_page;
    remove_pos_in_page = pos % count_in_page;
    assert(remove_page_pos < entry->m_page_count);

    assert(*count > 0);

    last_page_pos = ((*count) - 1) / count_in_page;
    last_pos_in_page = ((*count) - 1) % count_in_page;
    assert(last_page_pos < entry->m_page_count);

    oid = ((gd_om_oid_t *)obj) + entry->m_page_begin + remove_page_pos;
    assert(*oid != GD_OM_INVALID_OID);

    page_buf = ((char*)gd_om_obj_get(mgr->m_omm, *oid, mgr->m_em));
    assert(page_buf);

    while(remove_page_pos < last_page_pos) {
        char * next_page_buf;

        if (remove_pos_in_page + 1 < count_in_page) {
            memmove(
                page_buf + element_size * remove_pos_in_page,
                page_buf + element_size * (remove_pos_in_page + 1),
                element_size * (count_in_page - remove_pos_in_page - 1));
        }

        ++oid;
        assert(*oid != GD_OM_INVALID_OID);

        next_page_buf = ((char*)gd_om_obj_get(mgr->m_omm, *oid, mgr->m_em));
        assert(next_page_buf);

        memmove(page_buf + element_size * (count_in_page - 1), next_page_buf, element_size);

        page_buf = next_page_buf;
        ++remove_page_pos;
        remove_pos_in_page = 0;
    };

    last_page_left_count =
        (*count)
        - (count_in_page * remove_page_pos)
        - remove_pos_in_page
        - 1;

    if (last_page_left_count > 0) {
        memmove(
            page_buf + element_size * remove_pos_in_page,
            page_buf + element_size * (remove_pos_in_page + 1),
            element_size * last_page_left_count);
    }
    else {
        if (remove_pos_in_page == 0) {
            gd_om_obj_free(mgr->m_omm, *oid, mgr->m_em);
            *oid = GD_OM_INVALID_OID;
        }
    }
    
    --(*count);
    return 0;
}

int om_grp_obj_list_sort_ex(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, om_grp_entry_meta_t entry, int (*cmp)(void const *, void const *)) {
    return 0;
}

void * om_grp_obj_list_bsearch_ex(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, om_grp_entry_meta_t entry, const void * key, int (*cmp)(void const *, void const *)) {
    return NULL;
}
