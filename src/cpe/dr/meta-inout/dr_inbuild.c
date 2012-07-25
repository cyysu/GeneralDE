#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/dr/dr_error.h"
#include "dr_inbuild.h"

static uint32_t dr_inbuild_macro_hash(const struct DRInBuildMacro * data) {
    return data->m_name ? cpe_hash_str(data->m_name, strlen(data->m_name)) : 1;
}

static int dr_inbuild_macro_cmp(const struct DRInBuildMacro * l, const struct DRInBuildMacro * r) {
    return strcmp(l->m_name, r->m_name) == 0;
}

static uint32_t dr_inbuild_meta_hash(const struct DRInBuildMeta * data) {
    return data->m_name ? cpe_hash_str(data->m_name, strlen(data->m_name)) : 1;
}

static int dr_inbuild_meta_cmp(const struct DRInBuildMeta * l, const struct DRInBuildMeta * r) {
    return strcmp(l->m_name, r->m_name) == 0;
}

struct DRInBuildMetaLib * dr_inbuild_create_lib(void) {
    struct DRInBuildMetaLib * inBuildMetaLib = (struct DRInBuildMetaLib *)malloc(sizeof(struct DRInBuildMetaLib));
    if (inBuildMetaLib == NULL) {
        return NULL;
    }

    bzero(inBuildMetaLib, sizeof(*inBuildMetaLib));

    mem_buffer_init(&inBuildMetaLib->m_tmp_buf, NULL);

    if (cpe_hash_table_init(
            &inBuildMetaLib->m_index_macros, 
            NULL,
            (cpe_hash_fun_t) dr_inbuild_macro_hash,
            (cpe_hash_cmp_t) dr_inbuild_macro_cmp,
            CPE_HASH_OBJ2ENTRY(DRInBuildMacro, m_hh),
            -1) != 0)
    {
        mem_buffer_clear(&inBuildMetaLib->m_tmp_buf);
        free(inBuildMetaLib);
        return NULL;
    }

    if (cpe_hash_table_init(
            &inBuildMetaLib->m_index_metas, 
            NULL,
            (cpe_hash_fun_t) dr_inbuild_meta_hash,
            (cpe_hash_cmp_t) dr_inbuild_meta_cmp,
            CPE_HASH_OBJ2ENTRY(DRInBuildMeta, m_hh),
            -1) != 0)
    {
        mem_buffer_clear(&inBuildMetaLib->m_tmp_buf);
        cpe_hash_table_fini(&inBuildMetaLib->m_index_macros);
        free(inBuildMetaLib);
        return NULL;
    }


    inBuildMetaLib->m_data.iID = -1;
    
    TAILQ_INIT(&inBuildMetaLib->m_macros);

    TAILQ_INIT(&inBuildMetaLib->m_metas);

    return inBuildMetaLib;
}

void dr_inbuild_free_lib(struct DRInBuildMetaLib * inBuildMetaLib) {
    if (inBuildMetaLib == NULL) {
        return;
    }

    /*free macro list*/
    while(! TAILQ_EMPTY(&inBuildMetaLib->m_macros)) {
        dr_inbuild_metalib_remove_macro(inBuildMetaLib, TAILQ_FIRST(&inBuildMetaLib->m_macros));
    }

    /*free meta list*/
    while(! TAILQ_EMPTY(&inBuildMetaLib->m_metas)) {
        dr_inbuild_metalib_remove_meta(inBuildMetaLib, TAILQ_FIRST(&inBuildMetaLib->m_metas));
    }


    mem_buffer_clear(&inBuildMetaLib->m_tmp_buf);
    cpe_hash_table_fini(&inBuildMetaLib->m_index_macros);
    cpe_hash_table_fini(&inBuildMetaLib->m_index_metas);

    free(inBuildMetaLib);
}

void dr_inbuild_meta_clear(struct DRInBuildMeta * meta) {
    /*free meta list*/
    while(! TAILQ_EMPTY(&meta->m_entries)) {
        dr_inbuild_meta_remove_entry(meta, TAILQ_FIRST(&meta->m_entries));
    }
}

struct DRInBuildMacro * dr_inbuild_metalib_add_macro(struct DRInBuildMetaLib * inBuildMetaLib) {
    struct DRInBuildMacro * newMacro = (struct DRInBuildMacro *)malloc(sizeof(struct DRInBuildMacro));
    if (newMacro == NULL) {
        return NULL;
    }

    bzero(newMacro, sizeof(struct DRInBuildMacro));
    cpe_hash_entry_init(&newMacro->m_hh);

    TAILQ_INSERT_TAIL(&inBuildMetaLib->m_macros, newMacro, m_next);

    return newMacro;
}

int dr_inbuild_metalib_add_macro_to_index(struct DRInBuildMetaLib * inBuildMetaLib, struct DRInBuildMacro * macro) {
    return cpe_hash_table_insert_unique(&inBuildMetaLib->m_index_macros, macro);
}

void dr_inbuild_metalib_remove_macro(struct DRInBuildMetaLib * inBuildMetaLib, struct DRInBuildMacro * macro) {
    if (macro == NULL) return;

    if (cpe_hash_table_find(&inBuildMetaLib->m_index_macros, macro) == macro) {
        cpe_hash_table_remove_by_ins(&inBuildMetaLib->m_index_macros, macro);
    }

    TAILQ_REMOVE(&inBuildMetaLib->m_macros, macro, m_next);

    free(macro);
}

struct DRInBuildMacro *
dr_inbuild_metalib_find_macro(
    struct DRInBuildMetaLib * inBuildMetaLib, const char * macro_name)
{
    struct DRInBuildMacro key;

    key.m_name = (char *)macro_name;

    return (struct DRInBuildMacro *)cpe_hash_table_find(&inBuildMetaLib->m_index_macros, &key);
}

struct DRInBuildMeta *
dr_inbuild_metalib_add_meta(struct DRInBuildMetaLib * inBuildMetaLib) {
    struct DRInBuildMeta * newMeta = (struct DRInBuildMeta *)malloc(sizeof(struct DRInBuildMeta));
    if (newMeta == NULL) {
        return NULL;
    }

    bzero(newMeta, sizeof(struct DRInBuildMeta));
    newMeta->m_data.m_id = -1;

    TAILQ_INIT(&newMeta->m_entries);

    TAILQ_INSERT_TAIL(&inBuildMetaLib->m_metas, newMeta, m_next);

    return newMeta;
}

void dr_inbuild_metalib_remove_meta(struct DRInBuildMetaLib * inBuildMetaLib, struct DRInBuildMeta * meta) {
    if (meta == NULL) {
        return;
    }

    dr_inbuild_meta_clear(meta);

    TAILQ_REMOVE(&inBuildMetaLib->m_metas, meta, m_next);

    free(meta);
}

void dr_inbuild_meta_set_type(struct DRInBuildMeta * meta, int type) {
    meta->m_data.m_type = type;
}

void dr_inbuild_meta_set_id(struct DRInBuildMeta * meta, int id) {
    meta->m_data.m_id = id;
}

void dr_inbuild_meta_set_align(struct DRInBuildMeta * meta, int align) {
    meta->m_data.m_align = align;
}

void dr_inbuild_meta_set_base_version(struct DRInBuildMeta * meta, int version) {
    meta->m_data.m_based_version = version;
}

int dr_inbuild_meta_current_version(struct DRInBuildMeta * meta) {
    return meta->m_data.m_current_version;
}

void dr_inbuild_meta_set_current_version(struct DRInBuildMeta * meta, int version) {
    meta->m_data.m_current_version = version;
}

void dr_inbuild_meta_set_name(struct DRInBuildMeta * meta, const char * name) {
    meta->m_name = name;
}

void dr_inbuild_meta_set_desc(struct DRInBuildMeta * meta, const char * desc) {
    meta->m_desc = desc;
}

struct DRInBuildMetaEntry *
dr_inbuild_meta_add_entry(struct DRInBuildMeta * meta) {
    struct DRInBuildMetaEntry * newEntry = (struct DRInBuildMetaEntry *)malloc(sizeof(struct DRInBuildMetaEntry));
    if (newEntry == NULL) {
        return NULL;
    }

    bzero(newEntry, sizeof(struct DRInBuildMetaEntry));
    newEntry->m_data.m_id = -1;
    newEntry->m_data.m_array_count = 1;
    newEntry->m_data.m_array_refer_data_start_pos = -1;
    newEntry->m_data.m_array_refer_entry_pos = -1;
    newEntry->m_data.m_type = CPE_DR_TYPE_UNKOWN;
    newEntry->m_data.m_select_range_min = 1;
    newEntry->m_data.m_select_data_start_pos = -1;
    newEntry->m_data.m_select_entry_pos = -1;
    newEntry->m_data.m_dft_value_pos = -1;

    TAILQ_INSERT_TAIL(&meta->m_entries, newEntry, m_next);
    ++meta->m_entries_count;

    return newEntry;
}

void dr_inbuild_meta_remove_entry(struct DRInBuildMeta * meta, struct DRInBuildMetaEntry * entry) {
    if (entry == NULL) {
        return;
    }

    TAILQ_REMOVE(&meta->m_entries, entry, m_next);
    --meta->m_entries_count;

    free(entry);
}

int dr_inbuild_entry_version(struct DRInBuildMetaEntry * entry) {
    return entry->m_data.m_version;
}

void dr_inbuild_entry_set_type(struct DRInBuildMetaEntry * entry, const char * type_name) {
    entry->m_ref_type_name = type_name;
}

void dr_inbuild_entry_set_id(struct DRInBuildMetaEntry * entry, int id) {
    entry->m_data.m_id = id;
}

void dr_inbuild_entry_set_version(struct DRInBuildMetaEntry * entry, int version) {
    entry->m_data.m_version = version;
}

void dr_inbuild_entry_set_array_count(struct DRInBuildMetaEntry * entry, int array_count) {
    entry->m_data.m_array_count = array_count;
}

void dr_inbuild_entry_set_name(struct DRInBuildMetaEntry * entry, const char * name) {
    entry->m_name = name;
}

void dr_inbuild_entry_set_desc(struct DRInBuildMetaEntry * entry, const char * desc) {
    entry->m_desc = desc;
}

