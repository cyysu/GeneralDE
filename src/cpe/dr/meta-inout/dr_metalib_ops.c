#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_error.h"
#include "../dr_ctype_ops.h"
#include "dr_metalib_ops.h"
#include "dr_inbuild_error.h"

int dr_lib_init(LPDRMETALIB pstLib, const LPDRLIBPARAM pstParam) {
    bzero(pstLib, pstParam->iSize);

    pstLib->m_magic = CPE_DR_MAGIC;
    pstLib->m_build_version = 11;
    //int8_t reserve1[4];
    pstLib->m_size = pstParam->iSize;
    //int8_t reserve2[16];
    pstLib->m_id = pstParam->iID;
    pstLib->m_tag_set_version = pstParam->iTagSetVersion;
    //int8_t reserve3[4];
    pstLib->m_meta_max_count = pstParam->iMaxMetas;
    pstLib->m_meta_count = 0;
    pstLib->m_macro_max_count = pstParam->iMaxMacros;
    pstLib->m_macro_count = 0;
    pstLib->m_macrogroup_max_count = pstParam->iMaxMacrosGroupNum;
    pstLib->m_macrogroup_count = 0;
    //int8_t reserve4[8];
    pstLib->m_version = pstParam->iVersion;
    pstLib->m_startpos_macro = 0; /*guess*/
    pstLib->m_startpos_meta_by_id = pstLib->m_startpos_macro + sizeof(struct tagDRMacro) * pstParam->iMaxMacros;
    pstLib->m_startpos_meta_by_name = pstLib->m_startpos_meta_by_id + sizeof(struct idx_meta_by_id) * pstParam->iMaxMetas;
    pstLib->m_startpos_meta_by_orig = pstLib->m_startpos_meta_by_name + sizeof(struct idx_meta_by_name) * pstParam->iMaxMetas;
    pstLib->m_startpos_meta = pstLib->m_startpos_meta_by_orig + sizeof(struct idx_meta_by_orig /*TODO: change type*/) * pstParam->iMaxMetas;
    pstLib->m_startpos_str = pstLib->m_startpos_meta + pstParam->iMetaSize;
    pstLib->m_buf_size_str = pstParam->iStrBufSize;
    //int8_t reserve7[8];
    pstLib->m_buf_size_meta = pstParam->iMetaSize;
    //int8_t reserve8[4];
    pstLib->m_buf_size_macros = sizeof(struct tagDRMacro) * pstParam->iMaxMacros;
    //int8_t reserve9[24];
    strncpy(pstLib->m_name, pstParam->szName, CPE_DR_NAME_LEN);

    return 0;
}

void * dr_lib_alloc_in_strbuf(
    LPDRMETALIB metaLib, size_t dataSize, int * usedCount, error_monitor_t em)
{
    void * p;

    if ((*usedCount + dataSize) > metaLib->m_buf_size_str) { /*overflow*/
        DR_NOTIFY_ERROR(em, CPE_DR_ERROR_NO_SPACE_FOR_STRBUF);
        return NULL;
    }

    p = (char *)(metaLib + 1) + metaLib->m_startpos_str + *usedCount;

    *usedCount += dataSize;

    return p;
}

LPDRMACRO dr_lib_add_macro(LPDRMETALIB metaLib, LPDRMACRO macro, error_monitor_t em) {
    char * base = (char*)(metaLib + 1);
    LPDRMACRO newMacro = NULL;

    if (metaLib->m_macro_count >= metaLib->m_macro_max_count) {
        return NULL;
    }

    newMacro = ((LPDRMACRO)(base + metaLib->m_startpos_macro)) + metaLib->m_macro_count;
    memcpy(newMacro, macro, sizeof(*macro));
    ++metaLib->m_macro_count;
    return newMacro;
}

static int dr_lib_find_next_meta_pos(LPDRMETALIB metaLib) {
    char * base = (char*)(metaLib + 1);
    int i = 0;

    struct idx_meta_by_id * metaIdx = (struct idx_meta_by_id *)(base + metaLib->m_startpos_meta_by_id);

    int newMetaPos = metaLib->m_startpos_meta;

    for(i = 0; i < metaLib->m_meta_count; ++i) {
        int curNextMetaPos = metaIdx[i].m_diff_to_base + ((LPDRMETA)(base + metaIdx[i].m_diff_to_base))->m_meta_size;
        if (curNextMetaPos > newMetaPos) {
            newMetaPos = curNextMetaPos;
        }
    }

    return newMetaPos;
}

static void dr_lib_add_meta_index_for_name(
    LPDRMETALIB metaLib, LPDRMETA newMeta, error_monitor_t em)
{
    char * base = (char*)(metaLib + 1);
    int beginPos, endPos, curPos;
    struct idx_meta_by_name * putAt = NULL;
    struct idx_meta_by_name * searchStart =
        (struct idx_meta_by_name *)(base + metaLib->m_startpos_meta_by_name);

    for(beginPos = 0, endPos = metaLib->m_meta_count, curPos = (endPos - beginPos - 1) / 2;
        beginPos < endPos;
        curPos = beginPos + (endPos - beginPos - 1) / 2)
    {
        struct idx_meta_by_name * curItem = searchStart + curPos;

        int cmp_result = strcmp(base + newMeta->m_name_pos, base + curItem->m_name_pos);
        if (cmp_result <= 0) {
            endPos = curPos;
        }
        else {
            beginPos = curPos + 1;
        }
    }

    putAt = searchStart + curPos;

    if (curPos < metaLib->m_meta_count) {
        if (strcmp(base + newMeta->m_name_pos, base + putAt->m_name_pos) == 0) {
            DR_NOTIFY_ERROR(em, CPE_DR_ERROR_META_NAME_CONFLICT);
        }

        memmove(
            searchStart + curPos + 1,
            searchStart + curPos,
            sizeof(struct idx_meta_by_name) * (metaLib->m_meta_count - curPos));
    }

    putAt->m_name_pos = newMeta->m_name_pos;
    putAt->m_diff_to_base = newMeta->m_self_pos;
}

static void dr_lib_add_meta_index_for_orig(
    LPDRMETALIB metaLib, LPDRMETA newMeta, error_monitor_t em)
{
    char * base = (char*)(metaLib + 1);
    struct idx_meta_by_orig * putAt = NULL;
    struct idx_meta_by_orig * begin =
        (struct idx_meta_by_orig *)(base + metaLib->m_startpos_meta_by_orig);

    putAt = begin + metaLib->m_meta_count;
    putAt->m_diff_to_base = newMeta->m_self_pos;
    putAt->m_reserve = 0;
}

static void dr_lib_add_meta_index_for_id(
    LPDRMETALIB metaLib, LPDRMETA newMeta, error_monitor_t em)
{
    char * base = (char*)(metaLib + 1);
    int beginPos, endPos, curPos;
    struct idx_meta_by_id * putAt = NULL;
    struct idx_meta_by_id * searchStart =
        (struct idx_meta_by_id *)(base + metaLib->m_startpos_meta_by_id);

    for(beginPos = 0, endPos = metaLib->m_meta_count, curPos = (endPos - beginPos - 1) / 2;
        beginPos < endPos;
        curPos = beginPos + (endPos - beginPos - 1) / 2)
    {
        struct idx_meta_by_id * curItem = searchStart + curPos;
        
        if (newMeta->m_id <= curItem->m_id) {
            endPos = curPos;
        }
        else {
            beginPos = curPos + 1;
        }
    }

    putAt = searchStart + curPos;

    if (curPos < metaLib->m_meta_count) {
        if (newMeta->m_id == putAt->m_id && newMeta->m_id >= 0) {
            DR_NOTIFY_ERROR(em, CPE_DR_ERROR_META_ID_CONFLICT);
        }

        memmove(
            searchStart + curPos + 1,
            searchStart + curPos,
            sizeof(struct idx_meta_by_id) * (metaLib->m_meta_count - curPos));
    }

    putAt->m_id = newMeta->m_id;
    putAt->m_diff_to_base = newMeta->m_self_pos;
}

LPDRMETA
dr_lib_add_meta(LPDRMETALIB metaLib, LPDRMETA meta, error_monitor_t em) {
    char * base = (char*)(metaLib + 1);
    LPDRMETA newMeta = NULL;
    int newMetaPos = 0;

    if (metaLib->m_meta_count >= metaLib->m_meta_max_count) {
        DR_NOTIFY_ERROR(em, CPE_DR_ERROR_NO_SPACE_FOR_MATA);
        return NULL;
    }

    newMetaPos = dr_lib_find_next_meta_pos(metaLib);

    if ( (newMetaPos + meta->m_meta_size)
         > (metaLib->m_startpos_meta + metaLib->m_buf_size_meta) )
    {
        DR_NOTIFY_ERROR(em, CPE_DR_ERROR_NO_SPACE_FOR_MATA);
        return NULL;
    }

    newMeta = (LPDRMETA)(base + newMetaPos);

    memcpy(newMeta, meta, sizeof(*newMeta));
    newMeta->m_self_pos = newMetaPos;
    newMeta->m_entry_count = 0;

    dr_lib_add_meta_index_for_name(metaLib, newMeta, em);
    dr_lib_add_meta_index_for_id(metaLib, newMeta, em);
    dr_lib_add_meta_index_for_orig(metaLib, newMeta, em);

    /*must inc m_meta_count here
      becuse dr_add_metalib_meta_add_index_by_xxx will use this */
    ++metaLib->m_meta_count;

    return newMeta;;
}

void dr_meta_add_entry_calc_align(
    LPDRMETA meta, LPDRMETAENTRY newEntry, int entryAlign,
    error_monitor_t em)
{
    int align = entryAlign < meta->m_align ? entryAlign : meta->m_align;

    if (meta->m_type == CPE_DR_TYPE_STRUCT) {
        int panding = meta->m_data_size % align;
        if (panding) {
            panding = align - panding;
        }

        newEntry->m_data_start_pos = meta->m_data_size + panding;
        meta->m_data_size += panding + newEntry->m_unitsize;
    }
    else if (meta->m_type == CPE_DR_TYPE_UNION) {
        newEntry->m_data_start_pos = 0;
        if (meta->m_data_size < newEntry->m_unitsize) {
            meta->m_data_size = newEntry->m_unitsize;
        }
    }
    else {
        CPE_ERROR_EX(em, CPE_DR_ERROR_ENTRY_INVALID_TYPE_VALUE, "unknown meta type %d!", (int)meta->m_type);
    }
}

int dr_add_meta_entry_set_type_calc_align(LPDRMETA meta, LPDRMETAENTRY entry, error_monitor_t em) {
    char * base = (char*)(meta) - meta->m_self_pos;
    int entryAlign = 0;

    //process type
    if (entry->m_type <= CPE_DR_TYPE_COMPOSITE) {/*is composite type*/
        LPDRMETA usedType = NULL;

        if (entry->m_ref_type_pos < 0) {
            DR_NOTIFY_ERROR(em, CPE_DR_ERROR_ENTRY_INVALID_TYPE_VALUE);
            return -1;
        }

        usedType = (LPDRMETA)(base + entry->m_ref_type_pos);

        if (usedType == NULL) {
            DR_NOTIFY_ERROR(em, CPE_DR_ERROR_ENTRY_INVALID_TYPE_VALUE);
            return -1;
        }

        entry->m_unitsize = usedType->m_data_size * (entry->m_array_count < 1 ? 1 : entry->m_array_count);
        entryAlign = usedType->m_align;
    }
    else { /* is basic type */
        const struct tagDRCTypeInfo * typeInfo = dr_find_ctype_info_by_type(entry->m_type);
        if (typeInfo == NULL) {
            DR_NOTIFY_ERROR(em, CPE_DR_ERROR_ENTRY_INVALID_TYPE_VALUE);
            return -1;
        }

        if ((int)typeInfo->m_size > 0) {
            entryAlign = typeInfo->m_size;
        }
        else {
            entryAlign = 1;
        }
    }

    return entryAlign;
}

LPDRMETAENTRY
dr_meta_add_entry(LPDRMETA meta, LPDRMETAENTRY entry, error_monitor_t em) {
    LPDRMETAENTRY newEntry =  (LPDRMETAENTRY)(meta + 1) + meta->m_entry_count;
    int entryAlign = 0;

    if (entry->m_name_pos < 0) {
        CPE_ERROR_EX(em, CPE_DR_ERROR_META_NO_NAME, "entry have no name!");
        return NULL;
    }

    //process type
    entryAlign = dr_add_meta_entry_set_type_calc_align(meta, entry, em);
    if (entryAlign < 0) {
        return NULL;
    }

    memcpy(newEntry, entry, sizeof(*newEntry));

    newEntry->m_self_to_meta_pos = (char*)newEntry - (char*)meta;

    dr_meta_add_entry_calc_align(meta, newEntry, entryAlign, em);

    if (newEntry->m_version > meta->m_current_version) {
        meta->m_current_version = newEntry->m_version;
    }

    ++meta->m_entry_count;
    return newEntry;
}

static LPDRMETAENTRY dr_meta_find_entry_lsearch(LPDRMETA meta, const char * name) {
    char * base = (char*)(meta) - meta->m_self_pos;
    uint32_t i;

    for(i = 0; i < meta->m_entry_count; ++i) {
        LPDRMETAENTRY entry =  (LPDRMETAENTRY)(meta + 1) + i;
        if (strcmp(base + entry->m_name_pos, name) == 0) {
            return entry;
        }
    }
    
    return NULL;
}

void dr_meta_add_key(LPDRMETA meta, const char * entry_name, error_monitor_t em) {
    dr_idx_entry_info_t entry_info;
    char * base = (char*)(meta) - meta->m_self_pos;
    uint32_t i;
    LPDRMETAENTRY entry;

    for(i = 0; i < meta->m_key_num; ++i) {
        entry_info = dr_meta_key_info_at(meta, i);
        LPDRMETAENTRY check_entry = (LPDRMETAENTRY)(base + entry_info->m_entry_diff_to_base);

        if (strcmp(base + check_entry->m_name_pos, entry_name) == 0) {
            CPE_ERROR_EX(em, CPE_DR_ERROR_META_NO_ENTRY, "meta %s have entry %s", dr_meta_name(meta), entry_name);
            return;
        }
    }

    entry = dr_meta_find_entry_lsearch(meta, entry_name);
    if (entry == NULL) {
        CPE_ERROR(em, "meta %s have entry %s", dr_meta_name(meta), entry_name);
        return;
    }
 
    ++meta->m_key_num;
    entry_info = dr_meta_key_info_at(meta, i);
    entry_info->m_entry_diff_to_base = ((char *)entry) - base;
    entry_info->m_data_start_pos = entry->m_data_start_pos;
}

void dr_meta_do_complete(LPDRMETA meta, error_monitor_t em) {
    int panding;

    if (meta->m_align == 0) meta->m_align = 1;

    panding = meta->m_data_size % meta->m_align;
    if (panding) {
        panding = meta->m_align - panding;
    }
    meta->m_data_size += panding;

    if (meta->m_entry_count == 0) {
        CPE_ERROR_EX(em, CPE_DR_ERROR_META_NO_ENTRY, "meta %s have no entry", dr_meta_name(meta));
    }
}

int dr_lib_addr_to_pos(LPDRMETALIB metaLib, const void * addr) {
    char * base = (char*)(metaLib + 1);
    assert(
        (const char *)addr >= base
        &&
        (const char *)addr < ((char *)metaLib) + metaLib->m_size);
    return (const char *)addr - base;
}
