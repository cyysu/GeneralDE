#include <assert.h>
#include "cpe/dr/dr_metalib_build.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/pom_grp/pom_grp_meta.h"
#include "cpe/pom_grp/pom_grp_store.h"
#include "pom_grp_internal_ops.h"

static int pom_grp_meta_build_store_meta_init_meta(struct DRInBuildMeta * new_meta, LPDRMETA template_meta, error_monitor_t em) {
    dr_inbuild_meta_set_name(new_meta, dr_meta_name(template_meta));
    dr_inbuild_meta_set_align(new_meta, 1);
    dr_inbuild_meta_set_id(new_meta, dr_meta_id(template_meta));
    dr_inbuild_meta_set_desc(new_meta, dr_meta_desc(template_meta));
    dr_inbuild_meta_set_type(new_meta, dr_meta_type(template_meta));
    return 0;
}

static int pom_grp_meta_build_store_meta_add_entry(struct DRInBuildMeta * new_meta, LPDRMETAENTRY template_entry, error_monitor_t em) {
    struct DRInBuildMetaEntry * new_entry = dr_inbuild_meta_add_entry(new_meta);
    if (new_entry == NULL) {
        CPE_ERROR(em, "pom_grp_meta_build_store_meta: create new entry %s fail!", dr_entry_name(template_entry));
        return -1;
    }

    dr_inbuild_entry_set_name(new_entry, dr_entry_name(template_entry));
    dr_inbuild_entry_set_id(new_entry, dr_entry_id(template_entry));
    dr_inbuild_entry_set_type(new_entry, dr_entry_type_name(template_entry));

    return 0;
}

static int pom_grp_meta_build_store_meta_on_entry_normal(
    struct DRInBuildMeta * meta,
    pom_grp_entry_meta_t entry,
    error_monitor_t em)
{
    int i, count;
    LPDRMETA entry_meta = pom_grp_entry_meta_normal_meta(entry);
    assert(entry_meta);

    count = dr_meta_entry_num(entry_meta);
    for(i = 0; i < count; ++i) {
        if (pom_grp_meta_build_store_meta_add_entry(meta, dr_meta_entry_at(entry_meta, i), em) != 0) return -1;
    }

    return 0;
}

static int pom_grp_meta_build_store_meta_on_entry_ba(
    struct DRInBuildMeta * meta,
    pom_grp_entry_meta_t entry,
    error_monitor_t em)
{
    struct DRInBuildMetaEntry * new_entry = dr_inbuild_meta_add_entry(meta);
    if (new_entry == NULL) {
        CPE_ERROR(em, "pom_grp_meta_build_store_meta: create new entry %s fail!", entry->m_name);
        return -1;
    }

    dr_inbuild_entry_set_name(new_entry, entry->m_name);
    dr_inbuild_entry_set_id(new_entry, -1);
    dr_inbuild_entry_set_type(new_entry, "uint8");
    dr_inbuild_entry_set_array_count(new_entry, pom_grp_entry_meta_ba_bytes(entry));

    return 0;
}

static int pom_grp_meta_build_store_meta_on_entry_binary(
    struct DRInBuildMeta * meta,
    pom_grp_entry_meta_t entry,
    error_monitor_t em)
{
    struct DRInBuildMetaEntry * new_entry = dr_inbuild_meta_add_entry(meta);
    if (new_entry == NULL) {
        CPE_ERROR(em, "pom_grp_meta_build_store_meta: create new entry %s fail!", entry->m_name);
        return -1;
    }

    dr_inbuild_entry_set_name(new_entry, entry->m_name);
    dr_inbuild_entry_set_id(new_entry, -1);
    dr_inbuild_entry_set_type(new_entry, "uint8");
    dr_inbuild_entry_set_array_count(new_entry, pom_grp_entry_meta_binary_capacity(entry));

    return 0;
}

static int pom_grp_meta_build_store_meta_on_entry_list(
    struct DRInBuildMetaLib * builder,
    LPDRMETAENTRY key_entry,
    pom_grp_entry_meta_t entry,
    error_monitor_t em)
{
    struct DRInBuildMeta * meta;
    int i, count;
    LPDRMETA entry_meta = pom_grp_entry_meta_list_meta(entry);
    assert(entry_meta);

    meta = dr_inbuild_metalib_add_meta(builder);
    if (meta == NULL) {
        CPE_ERROR(em, "pom_grp_meta_build_store_meta: create meta for %s fail!", entry->m_name);
        return -1;
    }

    pom_grp_meta_build_store_meta_init_meta(meta, entry_meta, em);

    if (pom_grp_meta_build_store_meta_add_entry(meta, key_entry, em) != 0) return -1;

    count = dr_meta_entry_num(entry_meta);
    for(i = 0; i < count; ++i) {
        if (pom_grp_meta_build_store_meta_add_entry(meta, dr_meta_entry_at(entry_meta, i), em) != 0) return -1;
    }

    return 0;
}

static int pom_grp_meta_build_store_meta_i(
    struct DRInBuildMetaLib * builder,
    pom_grp_meta_t meta,
    const char * main_entry,
    const char * key,
    error_monitor_t em)
{
    struct DRInBuildMeta * main_meta;
    LPDRMETAENTRY key_entry;
    pom_grp_entry_meta_t main_entry_meta;
    uint16_t i, count;

    main_entry_meta = pom_grp_entry_meta_find(meta, main_entry);
    if (main_entry_meta == NULL) {
        CPE_ERROR(em, "pom_grp_meta_build_store_meta: create metalib builder fail!");
        return -1;
    }

    if (main_entry_meta->m_type != pom_grp_entry_type_normal) {
        CPE_ERROR(
            em, "pom_grp_meta_build_store_meta: main entry %s is not type normal!",
            main_entry_meta->m_name);
        return -1;
    }

    key_entry = dr_meta_find_entry_by_name(pom_grp_entry_meta_normal_meta(main_entry_meta), key);
    if (key_entry == NULL) {
        CPE_ERROR(
            em, "pom_grp_meta_build_store_meta: key %s not exsit in %s(type=%s)!",
            key, main_entry_meta->m_name, dr_meta_name(pom_grp_entry_meta_normal_meta(main_entry_meta)));
        return -1;
    }

    main_meta = dr_inbuild_metalib_add_meta(builder);
    if (main_meta == NULL) {
        CPE_ERROR(em, "pom_grp_meta_build_store_meta: create main meta fail!");
        return -1;
    }

    if (pom_grp_meta_build_store_meta_init_meta(main_meta, pom_grp_entry_meta_normal_meta(main_entry_meta), em) != 0) return -1;

    if (pom_grp_meta_build_store_meta_on_entry_normal(main_meta, main_entry_meta, em) != 0) return -1;

    count = pom_grp_meta_entry_count(meta);
    for(i = 0; i < count; ++i) {
        pom_grp_entry_meta_t entry_meta = pom_grp_meta_entry_at(meta, i);
        if (entry_meta == main_entry_meta) continue;

        switch(entry_meta->m_type) {
        case pom_grp_entry_type_normal:
            if (pom_grp_meta_build_store_meta_on_entry_normal(main_meta, entry_meta, em) != 0) return -1;
            break;
        case pom_grp_entry_type_list:
            if (pom_grp_meta_build_store_meta_on_entry_list(builder, key_entry, entry_meta, em) != 0) return -1;
            break;
        case pom_grp_entry_type_ba:
            if (pom_grp_meta_build_store_meta_on_entry_ba(main_meta, entry_meta, em) != 0) return -1;
            break;
        case pom_grp_entry_type_binary:
            if (pom_grp_meta_build_store_meta_on_entry_binary(main_meta, entry_meta, em) != 0) return -1;
            break;
        }
    }

    return 0;
}

int pom_grp_meta_build_store_meta(
    mem_buffer_t buffer,
    pom_grp_meta_t meta,
    const char * main_entry,
    const char * key,
    error_monitor_t em)
{
    struct DRInBuildMetaLib * builder = dr_inbuild_create_lib();
    if (builder == NULL) {
        CPE_ERROR(em, "pom_grp_meta_build_store_meta: create metalib builder fail!");
        return -1;
    }

    if (pom_grp_meta_build_store_meta_i(builder, meta, main_entry, key, em) != 0) {
        return -1;
    }
    
    if (dr_inbuild_build_lib(buffer, builder, em) != 0) {
        CPE_ERROR(em, "pom_grp_meta_build_store_meta: build lib to buffer fail!");
        goto ERROR;
    }

    dr_inbuild_free_lib(builder);
    return 0;

ERROR:
    dr_inbuild_free_lib(builder);
    return -1;
}
