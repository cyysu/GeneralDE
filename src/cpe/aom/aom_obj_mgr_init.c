#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/buffer.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_metalib_init.h"
#include "cpe/dr/dr_metalib_build.h"
#include "cpe/pom/pom_manage.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "aom_internal_types.h"
#include "aom_data.h"

int aom_obj_mgr_buf_init(
    LPDRMETA meta,
    void * data, size_t data_capacity,
    error_monitor_t em)
{
    struct aom_obj_control_data * control;
    struct mem_buffer buffer;
    size_t base_size;
    size_t lib_size;
    size_t total_head_size;
    size_t size_tmp;
    struct DRInBuildMetaLib * inbuild_lib;
    LPDRMETALIB metalib;

    if (dr_meta_key_entry_num(meta) == 0) {
        CPE_ERROR(em, "aom_obj_buff_init: meta %s have no key!", dr_meta_name(meta));
        return -1;
    }

    inbuild_lib = dr_inbuild_create_lib();
    if (inbuild_lib == NULL) {
        CPE_ERROR(em, "aom_obj_buff_init: create inbuild metalib fail!");
        return -1;
    }

    if (dr_inbuild_metalib_copy_meta_r(inbuild_lib, meta) == NULL) {
        CPE_ERROR(em, "aom_obj_buff_init: copy meta fail!");
        dr_inbuild_free_lib(inbuild_lib);
        return -1;
    }

    if (dr_inbuild_tsort(inbuild_lib, em) != 0) {
        CPE_ERROR(em, "aom_obj_buff_init: sort meta fail!");
        dr_inbuild_free_lib(inbuild_lib);
        return -1;
    }

    mem_buffer_init(&buffer, NULL);
    if (dr_inbuild_build_lib(&buffer, inbuild_lib, em) != 0) {
        CPE_ERROR(em, "aom_obj_buff_init: build metalib fail!");
        mem_buffer_clear(&buffer);
        dr_inbuild_free_lib(inbuild_lib);
        return -1;
    }
    dr_inbuild_free_lib(inbuild_lib);

    metalib = (LPDRMETALIB)mem_buffer_make_continuous(&buffer, 0);

    base_size = sizeof(struct aom_obj_control_data);
    CPE_PAL_ALIGN_DFT(base_size);

    lib_size = dr_lib_size(metalib);
    CPE_PAL_ALIGN_DFT(lib_size);

    total_head_size = base_size + lib_size;

    if (total_head_size >= data_capacity) {
        CPE_ERROR(
            em, "aom_obj_buff_init: data buf too small! require "FMT_SIZE_T", but only "FMT_SIZE_T""
            ": control size "FMT_SIZE_T", metalib size "FMT_SIZE_T"",
            total_head_size, data_capacity, base_size, lib_size);
        mem_buffer_clear(&buffer);
        return -1;
    }

    control = (struct aom_obj_control_data *)data;

    control->m_magic = OM_GRP_OBJ_CONTROL_MAGIC;
    control->m_head_version = 1;
    strncpy(control->m_meta_name, dr_meta_name(meta), sizeof(control->m_meta_name));

    size_tmp = sizeof(struct aom_obj_control_data);
    CPE_PAL_ALIGN_DFT(size_tmp);
    control->m_metalib_start = size_tmp;
    control->m_metalib_size = dr_lib_size(metalib);
    memcpy(((char *)data) + control->m_metalib_start, (void *)metalib, control->m_metalib_size);

    size_tmp = control->m_metalib_size;
    CPE_PAL_ALIGN_DFT(size_tmp);
    control->m_data_start = control->m_metalib_start + size_tmp;
    control->m_data_size = data_capacity - control->m_data_start;
    bzero(((char *)data) + control->m_data_start, control->m_data_size);

    mem_buffer_clear(&buffer);

    return 0;
}

