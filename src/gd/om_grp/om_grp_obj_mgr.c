#include "cpe/pal/pal_stdio.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/om/om_manage.h"
#include "gd/om_grp/om_grp_obj_mgr.h"
#include "gd/om_grp/om_grp_meta.h"
#include "om_grp_internal_ops.h"
#include "om_grp_data.h"

om_grp_obj_mgr_t
om_grp_obj_mgr_create(
    mem_allocrator_t alloc,
    void * data,
    size_t data_capacity,
    error_monitor_t em)
{
    om_grp_obj_mgr_t obj_mgr;

    struct om_grp_obj_control_data * control;

    if (data_capacity < sizeof(struct om_grp_obj_control_data)) {
        CPE_ERROR(
            em, "om_grp_obj_mgr_create_by_attach: not enouth size, size="FMT_SIZE_T", control_data_size="FMT_SIZE_T"!",
            data_capacity, sizeof(struct om_grp_obj_control_data));
        return NULL;
    }

    control = (struct om_grp_obj_control_data *)data;

    if(control->m_magic != OM_GRP_OBJ_CONTROL_MAGIC) {
        CPE_ERROR(
            em, "om_grp_obj_mgr_create_by_attach: matic mismatch!, %d and %d",
            control->m_magic, OM_GRP_OBJ_CONTROL_MAGIC);
        return NULL;
    }

    if(control->m_head_version != 1) {
        CPE_ERROR(
            em, "om_grp_obj_mgr_create_by_attach: not support version %d",
            control->m_head_version);
        return NULL;
    }

    if (data_capacity < (control->m_objmeta_start + control->m_objmeta_size)) {
        CPE_ERROR(
            em, "om_grp_obj_mgr_create_by_attach: not enouth size, size="FMT_SIZE_T", objmeta end at %u!",
            data_capacity, control->m_objmeta_start + control->m_objmeta_size)
        return NULL;
    }

    if (data_capacity < (control->m_metalib_start + control->m_metalib_size)) {
        CPE_ERROR(
            em, "om_grp_obj_mgr_create_by_attach: not enouth size, size="FMT_SIZE_T", metalib end at %u!",
            data_capacity, control->m_metalib_start + control->m_metalib_size)
        return NULL;
    }

    if (data_capacity < (control->m_data_start + control->m_data_size)) {
        CPE_ERROR(
            em, "om_grp_obj_mgr_create_by_attach: not enouth size, size="FMT_SIZE_T", data end at %u!",
            data_capacity, control->m_data_start + control->m_data_size)
        return NULL;
    }

    obj_mgr = (om_grp_obj_mgr_t)mem_alloc(alloc, sizeof(struct om_grp_obj_mgr));
    if (obj_mgr == NULL) {
        CPE_ERROR(em, "om_grp_obj_mgr_create_by_attach: create fail!");
        return NULL;
    }

    obj_mgr->m_alloc = alloc;
    obj_mgr->m_full_base = (char *)data;
    obj_mgr->m_full_capacity = data_capacity;

    obj_mgr->m_meta =
        om_grp_entry_meta_build_from_bin(
            alloc,
            obj_mgr->m_full_base + control->m_objmeta_start,
            control->m_objmeta_size);

    obj_mgr->m_metalib = (LPDRMETALIB)(obj_mgr->m_full_base + control->m_metalib_start);

    obj_mgr->m_data_base = obj_mgr->m_full_base + control->m_data_start;
    obj_mgr->m_data_capacity = control->m_data_size;

    if (obj_mgr->m_meta == NULL) {
        CPE_ERROR(em, "om_grp_obj_mgr_create_by_attach: create meta fail!");
        mem_free(alloc, obj_mgr);
        return NULL;
    }

    obj_mgr->m_omm =
        gd_om_mgr_create(
            alloc,
            control->m_page_size,
            control->m_buffer_size);
    if (obj_mgr->m_omm == NULL) {
        CPE_ERROR(em, "om_grp_obj_mgr_create_by_attach: create omm fail!");
        om_grp_meta_free(obj_mgr->m_meta);
        mem_free(alloc, obj_mgr);
        return NULL;
    }

    return obj_mgr;
}

void om_grp_obj_mgr_free(om_grp_obj_mgr_t mgr) {
    gd_om_mgr_free(mgr->m_omm);
    om_grp_meta_free(mgr->m_meta);
    mem_free(mgr->m_alloc, mgr);
}

gd_om_mgr_t om_grp_obj_mgr_omm(om_grp_obj_mgr_t mgr) {
    return mgr->m_omm;
}
