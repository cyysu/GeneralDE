#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/dr/dr_ctypes_op.h"
#include "cpe/cfg/cfg_manage.h"
#include "cpe/cfg/cfg_read.h"
#include "cfg_internal_ops.h"

struct cfg_bin_read_stack {
    cfg_t m_output;
    uint32_t m_end_pos;
};

#define CFG_READ_BIN_READ_TYPE(__type) \
    if (read_pos + 1 > head->m_data_capacity) {            \
        CPE_ERROR(                                                          \
            em, "cfg_read: read type overflow, read_pos=%d, capacity=%d", \
            read_pos, head->m_data_capacity);                           \
        return -1;                                                      \
    }                                                       \
    __type = *(uint8_t*)(buf_data + read_pos);              \
    read_pos += 1                                           \

#define CFG_READ_BIN_READ_SIZE(__size) \
    if (read_pos + 4 > head->m_data_capacity) {            \
        CPE_ERROR(                                                          \
            em, "cfg_read: read size overflow, read_pos=%d, capacity=%d", \
            read_pos, head->m_data_capacity);                           \
        return -1;                                                      \
    }                                                       \
    __size = *(uint32_t*)(buf_data + read_pos);             \
    read_pos += 4                                           \

#define CFG_READ_BIN_READ_STR(__str) \
    if (read_pos + 4 > head->m_data_capacity) {            \
        CPE_ERROR(                                                          \
            em, "cfg_read: read str pos overflow, read_pos=%d, capacity=%d", \
            read_pos, head->m_data_capacity);                           \
        return -1;                                                      \
    }                                                       \
    tmp_str_pos = *(uint32_t*)(buf_data + read_pos);                    \
    if (tmp_str_pos >= head->m_strpool_capacity) {\
        CPE_ERROR(                                                          \
            em, "cfg_read: read str data overflow, str_pos=%d, capacity=%d", \
            tmp_str_pos, head->m_strpool_capacity);                           \
        return -1;                                                      \
    }                                                                   \
    read_pos += 4;                                                      \
    __str = buf_str + tmp_str_pos


#define CFG_READ_BIN_READ_VALUE(__type, __value)            \
        tmp_capacity = dr_type_size(__type);                                \
        if (read_pos + tmp_capacity > head->m_data_capacity) {              \
            CPE_ERROR(                                                      \
                em, "cfg_read: read type %s overflow, read_pos=%d, data-size=%d, capacity=%d", \
                dr_type_name(__type), read_pos, tmp_capacity, head->m_data_capacity); \
            return -1;                                                      \
        }                                                                   \
        if (dr_ctype_set_from_ctype(cfg_data(__value), cfg_type(__value), __type, buf_data + read_pos, em) != 0) { \
            CPE_ERROR(em, "cfg_read: read type %d fail", __type);           \
            return -1;                                                      \
        }                                                                   \
        read_pos += tmp_capacity


int cfg_read_bin_with_name(cfg_t cfg, const char * name, void const * input, size_t input_size, error_monitor_t em) {
    struct cfg_bin_read_stack processStack[64];
    struct cfg_format_bin_head * head;
    char const * buf_data;
    char const * buf_str;
    int tmp_capacity;
    uint32_t tmp_str_pos;
    int cur_type;
    int read_pos;
    int stackPos;

    assert(input);

    if (input_size < sizeof(struct cfg_format_bin_head)) {
        CPE_ERROR(em, "cfg_read: input size too small!");
        return -1;
    }

    head = (struct cfg_format_bin_head *)input;
    if (head->m_magic != CFG_FORMAT_BIN_MATIC) {
        CPE_ERROR(em, "cfg_read: head.magic mismatch!");
        return -1;
    }

    buf_data = ((char const *)input) + head->m_data_start;
    buf_str = ((char const *)input) + head->m_strpool_start;

    read_pos = 0;

    CFG_READ_BIN_READ_TYPE(cur_type);

    if (cur_type == CPE_CFG_TYPE_STRUCT) {
        processStack[0].m_output = name ? cfg_struct_add_struct(cfg, name, cfg_merge_use_new) : cfg;
        if (processStack[0].m_output == NULL) {
            CPE_ERROR(em, "cfg_bin_read: add root struct %s fail!", name);
            return -1;
        }
        CFG_READ_BIN_READ_SIZE(processStack[0].m_end_pos);
        stackPos = 0;
    }
    else if (cur_type == CPE_CFG_TYPE_SEQUENCE) {
        processStack[0].m_output = name ? cfg_struct_add_seq(cfg, name, cfg_merge_use_new) : cfg;
        if (processStack[0].m_output == NULL) {
            CPE_ERROR(em, "cfg_bin_read: add root seq %s fail!", name);
            return -1;
        }
        CFG_READ_BIN_READ_SIZE(processStack[0].m_end_pos);
        stackPos = 0;
    }
    else if (cur_type == CPE_CFG_TYPE_STRING) {
        const char * str;
        CFG_READ_BIN_READ_STR(str);

        if (name) {
            cfg_struct_add_string(cfg, name, str, cfg_merge_use_new);
        }
        else {
            if (dr_ctype_set_from_string(cfg_data(cfg), cfg_type(cfg), str, em) != 0) {
                CPE_ERROR(em, "cfg_bin_read: set from string fail");
                return -1;
            }
        }
        stackPos = -1;
    }
    else {
        if (name) {
            cfg_t tmp = cfg_struct_add_value(cfg, name, cur_type, cfg_merge_use_new);
            if (tmp == NULL) {
                CPE_ERROR(em, "cfg_bin_read: add value %s type %s fail", name, dr_type_name(cur_type));
                return -1;
            }
            CFG_READ_BIN_READ_VALUE(cur_type, tmp);
        }
        else {
            CFG_READ_BIN_READ_VALUE(cur_type, cfg);
        }

        stackPos = -1;
    }

    while(stackPos >= 0) {
        struct cfg_bin_read_stack * cur_stack;

    READ_GO_NEXT:

        assert(stackPos < CPE_ARRAY_SIZE(processStack));

        cur_stack = &processStack[stackPos];

        switch(cfg_type(cur_stack->m_output)) {
        case CPE_CFG_TYPE_SEQUENCE: {
            while(read_pos < cur_stack->m_end_pos) {
                CFG_READ_BIN_READ_TYPE(cur_type);

                if (cur_type == CPE_CFG_TYPE_STRING) {
                    const char * value;
                    CFG_READ_BIN_READ_STR(value);
                    if (cfg_seq_add_string(cur_stack->m_output, value) == NULL) {
                        CPE_ERROR(em, "cfg_bin_read: seq add str %s fail", value);
                        return -1;
                    }
                }
                else if (cfg_type_is_value(cur_type)) {
                    cfg_t tmp = cfg_seq_add_value(cur_stack->m_output, cur_type);
                    if (tmp == NULL) {
                        CPE_ERROR(em, "cfg_bin_read: seq add type %s fail", dr_type_name(cur_type));
                        return -1;
                    }
                    CFG_READ_BIN_READ_VALUE(cur_type, tmp);
                }
                else {
                    if (stackPos + 1 >= CPE_ARRAY_SIZE(processStack)) {
                        CPE_ERROR(em, "cfg_bin_read: max level reached!");
                        return -1;
                    }
                    else {
                        struct cfg_bin_read_stack * new_stack;

                        stackPos++;
                        new_stack = processStack + stackPos;
                        new_stack->m_output =
                            cur_type == CPE_CFG_TYPE_SEQUENCE
                            ? cfg_seq_add_seq(cur_stack->m_output)
                            : cfg_seq_add_struct(cur_stack->m_output);
                        
                        CFG_READ_BIN_READ_SIZE(new_stack->m_end_pos);

                        goto READ_GO_NEXT;
                    }
                }
            }
            break;
        }
        case CPE_CFG_TYPE_STRUCT: {
            const char * attr_name;

            while(read_pos < cur_stack->m_end_pos) {
                CFG_READ_BIN_READ_TYPE(cur_type);
                CFG_READ_BIN_READ_STR(attr_name);

                if (cur_type == CPE_CFG_TYPE_STRING) {
                    const char * value;
                    CFG_READ_BIN_READ_STR(value);
                    if (cfg_struct_add_string(cur_stack->m_output, attr_name, value, cfg_merge_use_new) == NULL) {
                        CPE_ERROR(em, "cfg_bin_read: add attr %s str %s fail", attr_name, value);
                        return -1;
                    }
                }
                else if (cfg_type_is_value(cur_type)) {
                    cfg_t tmp = cfg_struct_add_value(cur_stack->m_output, attr_name, cur_type, cfg_merge_use_new);
                    if (tmp == NULL) {
                        CPE_ERROR(em, "cfg_bin_read: add attr %s type %s fail", attr_name, dr_type_name(cur_type));
                        return -1;
                    }
                    CFG_READ_BIN_READ_VALUE(cur_type, tmp);
                }
                else {
                    if (stackPos + 1 >= CPE_ARRAY_SIZE(processStack)) {
                        CPE_ERROR(em, "cfg_bin_read: max level reached!");
                        return -1;
                    }
                    else {
                        struct cfg_bin_read_stack * new_stack;

                        stackPos++;
                        new_stack = processStack + stackPos;
                        new_stack->m_output =
                            cur_type == CPE_CFG_TYPE_SEQUENCE
                            ? cfg_struct_add_seq(cur_stack->m_output, attr_name, cfg_merge_use_new)
                            : cfg_struct_add_struct(cur_stack->m_output, attr_name, cfg_merge_use_new);
                        
                        CFG_READ_BIN_READ_SIZE(new_stack->m_end_pos);

                        goto READ_GO_NEXT;
                    }
                }
            }
            break;
        }
        default:
            CPE_ERROR(em, "cfg_bin_read: not support write basic value!");
            return -1;
        }

        --stackPos;
    }

    return 0;
}

 
int cfg_read_bin(cfg_t cfg, void const  * input, size_t input_len, error_monitor_t em) {
    return cfg_read_bin_with_name(cfg, NULL, input, input_len, em);
}
