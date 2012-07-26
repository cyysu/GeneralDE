#include "cpe/pal/pal_string.h"
#include "cpe/dr/dr_metalib_cmp.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "../dr_internal_types.h"

int dr_metalib_compatible_part(LPDRMETALIB full, LPDRMETALIB part) {
    int i, count;

    count = dr_lib_macro_num(part);
    for(i = 0; i < count; ++i) {
        LPDRMACRO l_macro = dr_lib_macro_at(part, i);
        LPDRMACRO r_macro = dr_lib_macro_find(full, dr_macro_name(part, l_macro));
        if (r_macro == NULL) return 0;
        if (l_macro->m_value != r_macro->m_value) return 0;
    }

    count = dr_lib_meta_num(part);
    for(i = 0; i < count; ++i) {
        LPDRMETA l_meta = dr_lib_meta_at(part, i);
        LPDRMETA r_meta = dr_lib_find_meta_by_name(full, dr_meta_name(l_meta));
        int entry_i, entry_count;

        if (r_meta == NULL) return 0;

        if (l_meta->m_type != r_meta->m_type
            || l_meta->m_id != r_meta->m_id
            || l_meta->m_data_size != r_meta->m_data_size
            || l_meta->m_entry_count != r_meta->m_entry_count
            || l_meta->m_align != r_meta->m_align)
            return 0;

        entry_count = l_meta->m_entry_count;
        for(entry_i = 0; entry_i < entry_count; ++entry_i) {
            LPDRMETAENTRY l_entry = dr_meta_entry_at(l_meta, entry_i);
            LPDRMETAENTRY r_entry = dr_meta_entry_at(r_meta, entry_i);

            if (l_entry->m_type != r_entry->m_type
                || l_entry->m_unitsize != r_entry->m_unitsize
                || l_entry->m_size != r_entry->m_size
                || l_entry->m_array_count != r_entry->m_array_count
                || l_entry->m_id != r_entry->m_id
                || strcmp(dr_entry_name(l_entry), dr_entry_name(r_entry)) != 0)
                return 0;
        }

    }

    return 1;
}
