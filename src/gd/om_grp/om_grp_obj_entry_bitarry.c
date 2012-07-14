#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/bitarry.h"
#include "gd/om/om_object.h"
#include "gd/om/om_manage.h"
#include "gd/om_grp/om_grp_meta.h"
#include "gd/om_grp/om_grp_obj.h"
#include "om_grp_internal_ops.h"

uint16_t om_grp_obj_ba_bit_capacity(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, const char * entry) {
    om_grp_entry_meta_t entry_meta = om_grp_entry_meta_find(mgr->m_meta, entry);
    if (entry_meta == NULL) {
        CPE_ERROR(mgr->m_em, "om_grp_obj_ba_bit_capacity: entry %s not exist!", entry);
        return 0;
    }

    return om_grp_obj_ba_bit_capacity_ex(mgr, obj, entry_meta);
}

uint16_t om_grp_obj_ba_byte_capacity(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, const char * entry) {
    om_grp_entry_meta_t entry_meta = om_grp_entry_meta_find(mgr->m_meta, entry);
    if (entry_meta == NULL) {
        CPE_ERROR(mgr->m_em, "om_grp_obj_ba_byte_capacity: entry %s not exist!", entry);
        return 0;
    }

    return om_grp_obj_ba_byte_capacity_ex(mgr, obj, entry_meta);
}

uint16_t om_grp_obj_ba_bit_count(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, const char * entry) {
    om_grp_entry_meta_t entry_meta = om_grp_entry_meta_find(mgr->m_meta, entry);
    if (entry_meta == NULL) {
        CPE_ERROR(mgr->m_em, "om_grp_obj_ba_count: entry %s not exist!", entry);
        return 0;
    }

    return om_grp_obj_ba_bit_count_ex(mgr, obj, entry_meta);
}

int om_grp_obj_ba_set_all(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, const char * entry, cpe_ba_value_t value) {
    om_grp_entry_meta_t entry_meta = om_grp_entry_meta_find(mgr->m_meta, entry);
    if (entry_meta == NULL) {
        CPE_ERROR(mgr->m_em, "om_grp_obj_ba_count: entry %s not exist!", entry);
        return -1;
    }

    return om_grp_obj_ba_set_all_ex(mgr, obj, entry_meta, value);
}

int om_grp_obj_ba_set(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, const char * entry, uint32_t pos, cpe_ba_value_t value) {
    om_grp_entry_meta_t entry_meta = om_grp_entry_meta_find(mgr->m_meta, entry);
    if (entry_meta == NULL) {
        CPE_ERROR(mgr->m_em, "om_grp_obj_ba_count: entry %s not exist!", entry);
        return -1;
    }

    return om_grp_obj_ba_set_ex(mgr, obj, entry_meta, pos, value);
}

cpe_ba_value_t om_grp_obj_ba_get(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, const char * entry, uint32_t pos) {
    om_grp_entry_meta_t entry_meta = om_grp_entry_meta_find(mgr->m_meta, entry);
    if (entry_meta == NULL) {
        CPE_ERROR(mgr->m_em, "om_grp_obj_ba_count: entry %s not exist!", entry);
        return -1;
    }

    return om_grp_obj_ba_get_ex(mgr, obj, entry_meta, pos);
}

uint16_t om_grp_obj_ba_bit_capacity_ex(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, om_grp_entry_meta_t entry) {
    assert(entry);
    assert(obj);
    assert(entry->m_type == om_grp_entry_type_ba);
    
    return entry->m_data.m_ba.m_bit_capacity;
}

uint16_t om_grp_obj_ba_byte_capacity_ex(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, om_grp_entry_meta_t entry) {
    assert(entry);
    assert(obj);
    assert(entry->m_type == om_grp_entry_type_ba);
    
    return entry->m_obj_size * entry->m_page_count;
}

uint16_t om_grp_obj_ba_bit_count_ex(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, om_grp_entry_meta_t entry) {
    uint16_t count;
    uint16_t i;
    gd_om_oid_t * oid;
    char * page_buf;

    assert(entry);
    assert(obj);
    assert(entry->m_type == om_grp_entry_type_ba);

    oid = ((gd_om_oid_t *)obj) + entry->m_page_begin;

    count = 0;
    for(i = 0; i < entry->m_page_count; ++i, ++oid) {
        if (*oid == GD_OM_INVALID_OID) continue;

        page_buf = ((char*)gd_om_obj_get(mgr->m_omm, *oid, mgr->m_em));
        count += cpe_ba_count((cpe_ba_t)page_buf, cpe_ba_bits_from_bytes(entry->m_obj_size));
    }

    return (uint16_t)count;
}

int om_grp_obj_ba_set_all_ex(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, om_grp_entry_meta_t entry, cpe_ba_value_t value) {
    uint16_t i;
    gd_om_oid_t * oid;
    char * page_buf;

    assert(entry);
    assert(obj);
    assert(entry->m_type == om_grp_entry_type_ba);

    oid = ((gd_om_oid_t *)obj) + entry->m_page_begin;

    if (value) {
        if (*oid == GD_OM_INVALID_OID) {
            *oid = gd_om_obj_alloc(mgr->m_omm, om_grp_entry_meta_name_hs(entry), mgr->m_em);
            if (*oid == GD_OM_INVALID_OID) {
                CPE_ERROR(mgr->m_em, "om_grp_obj_ba_set_all_ex: alloc %s buf fail!", entry->m_name);
                return -1;
            }
        }

        page_buf = ((char*)gd_om_obj_get(mgr->m_omm, *oid, mgr->m_em));
        memset(page_buf, entry->m_obj_size, 0xff);
    }
    else {
        for(i = 0; i < entry->m_page_count; ++i, ++oid) {
            if (*oid == GD_OM_INVALID_OID) continue;

            gd_om_obj_free(mgr->m_omm, *oid, mgr->m_em);
            *oid = GD_OM_INVALID_OID;
        }
    }

    return 0;
}

int om_grp_obj_ba_set_ex(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, om_grp_entry_meta_t entry, uint32_t pos, cpe_ba_value_t value) {
    uint16_t set_page_pos;
    uint32_t set_pos_in_page;
    gd_om_oid_t * oid;
    char * page_buf;

    assert(entry);
    assert(obj);
    assert(entry->m_type == om_grp_entry_type_ba);

    set_page_pos = cpe_ba_bytes_from_bits(pos) / entry->m_obj_size;
    assert(set_page_pos < entry->m_page_count);
    assert(cpe_ba_bits_from_bytes(set_page_pos * entry->m_obj_size) < pos);
    set_pos_in_page = pos - cpe_ba_bits_from_bytes(set_page_pos * entry->m_obj_size);

    oid = ((gd_om_oid_t *)obj) + entry->m_page_begin + set_page_pos;

    if (*oid == GD_OM_INVALID_OID) {
        if (value == cpe_ba_false) return 0;

        *oid = gd_om_obj_alloc(mgr->m_omm, om_grp_entry_meta_name_hs(entry), mgr->m_em);
        if (*oid == GD_OM_INVALID_OID) {
            CPE_ERROR(mgr->m_em, "om_grp_obj_ba_set_ex: alloc %s buf fail!", entry->m_name);
            return -1;
        }
    }

    page_buf = ((char*)gd_om_obj_get(mgr->m_omm, *oid, mgr->m_em));
    assert(page_buf);

    assert(value == cpe_ba_true);
    cpe_ba_set((cpe_ba_t)page_buf, set_pos_in_page, cpe_ba_true);

    return 0;
}

cpe_ba_value_t om_grp_obj_ba_get_ex(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, om_grp_entry_meta_t entry, uint32_t pos) {
    uint16_t get_page_pos;
    uint32_t get_pos_in_page;
    gd_om_oid_t * oid;
    char * page_buf;

    assert(entry);
    assert(obj);
    assert(entry->m_type == om_grp_entry_type_ba);

    get_page_pos = cpe_ba_bytes_from_bits(pos) / entry->m_obj_size;
    assert(get_page_pos < entry->m_page_count);
    assert(cpe_ba_bits_from_bytes(get_page_pos * entry->m_obj_size) < pos);
    get_pos_in_page = pos - cpe_ba_bits_from_bytes(get_page_pos * entry->m_obj_size);

    oid = ((gd_om_oid_t *)obj) + entry->m_page_begin + get_page_pos;
    if (*oid == GD_OM_INVALID_OID) return cpe_ba_false;

    page_buf = ((char*)gd_om_obj_get(mgr->m_omm, *oid, mgr->m_em));
    assert(page_buf);

    return cpe_ba_get((cpe_ba_t)page_buf, get_pos_in_page);
}
