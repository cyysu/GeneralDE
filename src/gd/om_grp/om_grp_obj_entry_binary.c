#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "gd/om/om_object.h"
#include "gd/om/om_manage.h"
#include "gd/om_grp/om_grp_meta.h"
#include "gd/om_grp/om_grp_obj.h"
#include "om_grp_internal_ops.h"

uint16_t om_grp_obj_binary_capacity(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, const char * entry) {
    om_grp_entry_meta_t entry_meta = om_grp_entry_meta_find(mgr->m_meta, entry);
    if (entry_meta == NULL) {
        CPE_ERROR(mgr->m_em, "om_grp_obj_binary_capacity: entry %s not exist!", entry);
        return 0;
    }

    return om_grp_obj_binary_capacity_ex(mgr, obj, entry_meta);
}

void * om_grp_obj_binary(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, const char * entry) {
    om_grp_entry_meta_t entry_meta = om_grp_entry_meta_find(mgr->m_meta, entry);
    if (entry_meta == NULL) {
        CPE_ERROR(mgr->m_em, "om_grp_obj_binary: entry %s not exist!", entry);
        return NULL;
    }

    return om_grp_obj_binary_ex(mgr, obj, entry_meta);
}

void * om_grp_obj_binary_check_or_create(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, const char * entry) {
    om_grp_entry_meta_t entry_meta = om_grp_entry_meta_find(mgr->m_meta, entry);
    if (entry_meta == NULL) {
        CPE_ERROR(mgr->m_em, "om_grp_obj_binary_check_or_create: entry %s not exist!", entry);
        return NULL;
    }

    return om_grp_obj_binary_check_or_create_ex(mgr, obj, entry_meta);
}

int om_grp_obj_binary_set(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, const char * entry, void * data) {
    om_grp_entry_meta_t entry_meta = om_grp_entry_meta_find(mgr->m_meta, entry);
    if (entry_meta == NULL) {
        CPE_ERROR(mgr->m_em, "om_grp_obj_binary_set_ex: entry %s not exist!", entry);
        return -1;
    }

    return om_grp_obj_binary_set_ex(mgr, obj, entry_meta, data);
}

uint16_t om_grp_obj_binary_capacity_ex(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, om_grp_entry_meta_t entry) {
    assert(entry);
    assert(obj);
    assert(entry->m_type == om_grp_entry_type_binary);

    return entry->m_obj_size;
}

void * om_grp_obj_binary_ex(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, om_grp_entry_meta_t entry) {
    gd_om_oid_t oid;

    assert(entry);
    assert(obj);
    assert(entry->m_type == om_grp_entry_type_binary);

    oid = ((gd_om_oid_t *)obj)[entry->m_page_begin];

    if (oid == GD_OM_INVALID_OID) return NULL;

    return gd_om_obj_get(mgr->m_omm, oid, mgr->m_em);
}

void * om_grp_obj_binary_check_or_create_ex(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, om_grp_entry_meta_t entry) {
    gd_om_oid_t oid;

    assert(entry);
    assert(obj);
    assert(entry->m_type == om_grp_entry_type_binary);

    oid = ((gd_om_oid_t *)obj)[entry->m_page_begin];

    if (oid == GD_OM_INVALID_OID) {
        oid = gd_om_obj_alloc(mgr->m_omm, om_grp_entry_meta_name_hs(entry), mgr->m_em);
        if (oid == GD_OM_INVALID_OID) {
            CPE_ERROR(mgr->m_em, "om_mgr_obj_binary_check_or_create_ex: alloc %s buf fail!", entry->m_name);
            return NULL;
        }
    }

    return gd_om_obj_get(mgr->m_omm, oid, mgr->m_em);
}

int om_grp_obj_binary_set_ex(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, om_grp_entry_meta_t entry, void * data) {
    gd_om_oid_t oid;
    void * r;

    assert(entry);
    assert(obj);
    assert(entry->m_type == om_grp_entry_type_binary);

    oid = ((gd_om_oid_t *)obj)[entry->m_page_begin];

    if (oid == GD_OM_INVALID_OID) {
        oid = gd_om_obj_alloc(mgr->m_omm, om_grp_entry_meta_name_hs(entry), mgr->m_em);
        if (oid == GD_OM_INVALID_OID) {
            CPE_ERROR(mgr->m_em, "om_mgr_obj_binary_set: alloc %s buf fail!", entry->m_name);
            return -1;
        }
    }

    r = gd_om_obj_get(mgr->m_omm, oid, mgr->m_em);
    assert(r);

    memcpy(r, data, entry->m_obj_size);

    return 0;
}
