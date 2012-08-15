#ifndef CPE_DR_INTERNALS_TYPES_H
#define CPE_DR_INTERNALS_TYPES_H
#include "cpe/pal/pal_stdio.h"
#include "cpe/dr/dr_types.h"
#include "cpe/dr/dr_ctypes_info.h"

#pragma pack(push,1)

#ifndef CPE_DR_METALIB_SIZE
#define CPE_DR_METALIB_SIZE 32
#endif

#if (CPE_DR_METALIB_SIZE == 64)
typedef int64_t dr_int_t;
#define FMT_DR_INT_T FMT_INT64_T
#else
typedef int32_t dr_int_t;
#define FMT_DR_INT_T FMT_INT32_T
#endif

struct tagDRMetaLib {
    /*000*/ /*000*/int16_t m_magic;
    /*002*/ /*000*/int16_t m_build_version;
    int8_t reserve1[4];
    /*008*/ /*008*/dr_int_t m_size;
    int8_t reserve2[16];
    /*028*/ /*032*/int32_t m_id;
    /*032*/ /*036*/int32_t m_tag_set_version;
    int8_t reserve3[4];
    /*040*/ /*044*/int32_t m_meta_max_count;
    /*044*/ /*048*/int32_t m_meta_count;
    /*048*/ /*052*/int32_t m_macro_max_count;
    /*052*/ /*056*/int32_t m_macro_count;
    /*056*/ /*060*/int32_t m_macrogroup_max_count;
    /*060*/ /*064*/int32_t m_macrogroup_count;
    int8_t reserve4[8];
    /*072*/ /*076*/int32_t m_version;
    /*076*/ dr_int_t m_startpos_macro; /*guess*/
    /*080*/ dr_int_t m_startpos_meta_by_id;
    /*084*/ dr_int_t m_startpos_meta_by_name;
    /*088*/ dr_int_t m_startpos_meta_by_orig; /*what is this index?*/
    /*092*/ dr_int_t m_startpos_meta;
    /*096*/ dr_int_t m_startpos_str;
    /*100*/ dr_int_t m_buf_size_str;
    dr_int_t reserve7[2];
    /*112*/ dr_int_t m_buf_size_meta;
    dr_int_t reserve8[1];
    /*120*/ dr_int_t m_buf_size_macros;
    dr_int_t reserve9[6];
    /*148*/ /*224*/char m_name[CPE_DR_NAME_LEN];
}; /**/ /*352*/

struct tagDRMeta {
    int8_t reserve_1[4];
    /*004*/ /*004*/ int32_t m_id;
    /*008*/ /*008*/ int32_t m_based_version;
    /*012*/ /*012*/ int32_t m_current_version;
    /*016*/ /*016*/ dr_int_t m_type;
    /*020*/ /*024*/ dr_int_t m_meta_size;
    /*024*/ /*032*/ dr_int_t reserve_2;
    /*028*/ /*040*/ dr_int_t m_data_size;
    int8_t reserve_3_1[8];
    dr_int_t reserve_3_2[1];


    /*044*/ /*064*/uint32_t m_entry_count;
    uint32_t m_index_count;
    dr_int_t m_index_pos_from_meta;
    dr_int_t reserve_4_2[1];
    /*060*/ /*088*/dr_int_t m_self_pos;
    int8_t reserve_5[16];
    /*080*/ /*112*/int32_t m_align;
    int8_t reserve_6_1[4];
    dr_int_t reserve_6_2[11];
    /*132*/ /*208*/dr_int_t m_name_pos;
    /*136*/ /*216*/dr_int_t m_desc_pos;
    dr_int_t reserve_7_1_1;
    int8_t reserve_7_1_2[6];
    /**/ /*238*/uint16_t m_key_num;
    dr_int_t reserve_7_2[3];
    dr_int_t m_key_idx_pos;
    dr_int_t reserve_7_3[4];
}; /**/  /*304*/

struct tagDRMetaEntry {
    /*000*/ /*000*/int32_t m_id;
    /*004*/ /*004*/int32_t m_version;
    /*008*/ /*008*/dr_int_t m_type;
    /*012*/ /*016*/dr_int_t m_name_pos;
    /*016*/ /*024*/dr_int_t m_unitsize;
    dr_int_t reserve_2[3];
    /*032*/ /*056*/dr_int_t m_size;
    /*036*/ /*064*/dr_int_t m_array_count;
    /*040*/ /*072*/dr_int_t m_data_start_pos_3; /*same as start pos?*/
    /*044*/ /*080*/dr_int_t m_data_start_pos;
    int8_t reserve_3_1[24];
    dr_int_t reserve_3_2[5];
    /*092*/ /*152*/dr_int_t m_array_refer_data_start_pos;
    /*096*/ /*160*/dr_int_t m_array_refer_entry_pos;
    dr_int_t reserve_4[1];
    /*104*/ /*176 ?*/dr_int_t m_select_data_start_pos;
    /*108*/ /*184*/dr_int_t m_select_entry_pos;
    int8_t reserve_5[8];
    /*120*/ /*200*/dr_int_t m_ref_type_pos;
    /*124*/ /*208*/int32_t m_select_range_max;
    /*128*/ /*212*/int32_t m_select_range_min;
    int8_t reserve_6[8];
    dr_int_t reserve_6_2[1];
    /*144*/ /*232*/dr_int_t m_desc_pos;
    /*148*/ /*240*/dr_int_t m_cname_pos;
    /*152*/ /*248*/dr_int_t m_dft_value_pos;
    dr_int_t reserve_7[2];
    /*164*/ /*272*/dr_int_t m_self_to_meta_pos;
    dr_int_t reserve_8[3];
};/*180*/ /*304*/

struct tagDRMacro {
    dr_int_t m_name_pos;
    dr_int_t m_value;
    dr_int_t m_desc_pos;
    dr_int_t reserve[1];
};

struct tagDRMacrosGroup {
    int8_t a;
};

struct dr_index_info {
    dr_int_t m_reserve_1;
    dr_int_t m_name_pos;
    dr_int_t m_diff_to_meta;
    dr_int_t m_entry_num;
    dr_int_t m_entry_start_pos_to_meta;
    dr_int_t m_reserve[5];
};

struct dr_index_entry_info {
    dr_int_t m_data_start_pos;
    dr_int_t m_entry_diff_to_base;
    dr_int_t m_reserve;
};

struct idx_meta_by_id {
    dr_int_t m_id;
    dr_int_t m_diff_to_base;
};

struct idx_meta_by_name {
    dr_int_t m_name_pos;
    dr_int_t m_diff_to_base;
};

struct idx_meta_by_orig {
    dr_int_t m_diff_to_base;
    dr_int_t m_reserve;
};

struct dr_idx_entry_info {
    dr_int_t m_data_start_pos;
    dr_int_t m_entry_diff_to_base;
};

#pragma pack(pop)

#endif
