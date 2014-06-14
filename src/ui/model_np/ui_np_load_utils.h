#ifndef UI_NP_LOAD_UTILS_H
#define UI_NP_LOAD_UTILS_H
#include "cpe/utils/file.h"
#include "cpe/utils/buffer.h"
#include "ui/model/ui_model_types.h"

char * ui_data_np_load_src_to_buff(mem_buffer_t buffer, ui_data_src_t src, error_monitor_t em);

int ui_data_np_find_attr_long(long * result, const char * attr_name, int nb_attributes, const void * attributes, error_monitor_t em);
int ui_data_np_find_attr_bool(uint8_t * result, const char * attr_name, int nb_attributes, const void * attributes, error_monitor_t em);
int ui_data_np_find_attr_float(float * result, const char * attr_name, int nb_attributes, const void * attributes, error_monitor_t em);
const char * ui_data_np_find_attr_string(
    char * buff, size_t capacity, const char * attr_name, int nb_attributes, const void * attributes, error_monitor_t em);

int ui_data_np_read_value_bool(uint8_t * result, const char * data, size_t data_len);
int ui_data_np_read_value_long(long * result, const char * data, size_t data_len);
int ui_data_np_read_value_float(float * result, const char * data, size_t data_len);

#define UI_NP_XML_READ_ATTR_INT(__type, __val, __attr_name)             \
    do {                                                                \
        long __value;                                                   \
        if (ui_data_np_find_attr_long(&__value, __attr_name, nb_attributes, attributes, ctx->m_em) != 0) { \
            return;                                                     \
        }                                                               \
        __val = (__type)(__value);                                      \
    } while(0)

#define UI_NP_XML_READ_ATTR_FLOAT(__val, __attr_name)                   \
    do {                                                                \
        if (ui_data_np_find_attr_float(&(__val), __attr_name, nb_attributes, attributes, ctx->m_em) != 0) { \
            return;                                                     \
        }                                                               \
    } while(0)


#define UI_NP_XML_READ_ATTR_BOOL(__val, __attr_name)                   \
    do {                                                                \
        if (ui_data_np_find_attr_bool(&(__val), __attr_name, nb_attributes, attributes, ctx->m_em) != 0) { \
            return;                                                     \
        }                                                               \
    } while(0)

#define UI_NP_XML_READ_ATTR_STRING(__val, __attr_name)                  \
    if (ui_data_np_find_attr_string(__val, sizeof(__val), __attr_name, nb_attributes, attributes, ctx->m_em) == NULL) { \
        return;                                                         \
    }

#define UI_NP_XML_READ_ATTR_VECTOR_2(__vec)                 \
    do {                                                    \
        UI_NP_XML_READ_ATTR_FLOAT((__vec)->value[0], "x"); \
        UI_NP_XML_READ_ATTR_FLOAT((__vec)->value[1], "y"); \
    } while(0)

#define UI_NP_XML_READ_ATTR_UNIT_VECTOR_2(__vec)        \
    do {                                                \
        UI_NP_XML_READ_ATTR_FLOAT((__vec)->x.k, "xk");   \
        UI_NP_XML_READ_ATTR_FLOAT((__vec)->x.b, "xb");  \
        UI_NP_XML_READ_ATTR_FLOAT((__vec)->y.k, "yk");  \
        UI_NP_XML_READ_ATTR_FLOAT((__vec)->y.b, "yb");  \
    } while(0)

#define UI_NP_XML_READ_ATTR_UNIT_RECT(__rect)        \
    do {                                                \
        UI_NP_XML_READ_ATTR_FLOAT((__rect)->lt.x.k, "ltk");   \
        UI_NP_XML_READ_ATTR_FLOAT((__rect)->lt.x.b, "ltb");  \
        UI_NP_XML_READ_ATTR_FLOAT((__rect)->lt.y.k, "tpk");  \
        UI_NP_XML_READ_ATTR_FLOAT((__rect)->lt.y.b, "tpb");  \
        UI_NP_XML_READ_ATTR_FLOAT((__rect)->rb.x.k, "rtk");   \
        UI_NP_XML_READ_ATTR_FLOAT((__rect)->rb.x.b, "rtb");  \
        UI_NP_XML_READ_ATTR_FLOAT((__rect)->rb.y.k, "bmk");  \
        UI_NP_XML_READ_ATTR_FLOAT((__rect)->rb.y.b, "bmb");  \
    } while(0)

#define UI_NP_XML_READ_ATTR_VECTOR_3(__vec)                 \
    do {                                                    \
        UI_NP_XML_READ_ATTR_FLOAT((__vec)->value[0], "x"); \
        UI_NP_XML_READ_ATTR_FLOAT((__vec)->value[1], "y"); \
        UI_NP_XML_READ_ATTR_FLOAT((__vec)->value[2], "z"); \
    } while(0)

#define UI_NP_XML_READ_ATTR_RECT(__rect)                        \
    do {                                                        \
        UI_NP_XML_READ_ATTR_INT(int32_t, (__rect)->lt, "LT");   \
        UI_NP_XML_READ_ATTR_INT(int32_t, (__rect)->tp, "TP");   \
        UI_NP_XML_READ_ATTR_INT(int32_t, (__rect)->rt, "RT");   \
        UI_NP_XML_READ_ATTR_INT(int32_t, (__rect)->bm, "BM");   \
    } while(0)

#define UI_NP_XML_READ_ATTR_COLOR(__color)              \
    do {                                                \
        UI_NP_XML_READ_ATTR_FLOAT((__color)->a, "A");   \
        UI_NP_XML_READ_ATTR_FLOAT((__color)->r, "R");   \
        UI_NP_XML_READ_ATTR_FLOAT((__color)->g, "G");   \
        UI_NP_XML_READ_ATTR_FLOAT((__color)->b, "B");   \
    } while(0)

#define UI_NP_XML_READ_VALUE_BOOL(__value)                              \
    if (ui_data_np_read_value_bool(&__value, (const char *)ch, (size_t)len) != 0) { \
        char buf[64];                                                   \
        if (len >= sizeof(buf)) len = sizeof(buf) - 1;                  \
        memcpy(buf, ch, len);                                           \
        buf[len] = 0;                                                   \
        CPE_ERROR(ctx->m_em, "%s: read bool from %s fail!", ctx->m_cur_tag_name, buf); \
        return;                                                         \
    }

#define UI_NP_XML_READ_VALUE_FLOAT(__value)                              \
    if (ui_data_np_read_value_float(&__value, (const char *)ch, (size_t)len) != 0) { \
        char buf[64];                                                   \
        if (len >= sizeof(buf)) len = sizeof(buf) - 1;                  \
        memcpy(buf, ch, len);                                           \
        buf[len] = 0;                                                   \
        CPE_ERROR(ctx->m_em, "%s: read float from %s fail!", ctx->m_cur_tag_name, buf); \
        return;                                                         \
    }

#define UI_NP_XML_READ_VALUE_STRING(__value)                            \
        if (len + 1 <= sizeof(__value)) {                               \
            memcpy(__value, ch, len);                                   \
            __value[len] = 0;                                           \
        }                                                               \
        else {                                                          \
            char buf[64];                                               \
            if (len >= sizeof(buf)) len = sizeof(buf) - 1;              \
            memcpy(buf, ch, len);                                       \
            buf[len] = 0;                                               \
            CPE_ERROR(ctx->m_em, "%s: read str from %s fail, overflow!",\
                      ctx->m_cur_tag_name, buf);                        \
            return;                                                     \
        }

#define UI_NP_XML_READ_VALUE_INT(__type, __value)                       \
        do {                                                            \
            long __tmp_v;                                               \
            if (ui_data_np_read_value_long(&__tmp_v, (const char *)ch, (size_t)len) != 0) { \
                char buf[64];                                           \
                if (len >= sizeof(buf)) len = sizeof(buf) - 1;          \
                memcpy(buf, ch, len);                                   \
                buf[len] = 0;                                           \
                CPE_ERROR(ctx->m_em, "%s: read long from %s fail!", ctx->m_cur_tag_name, buf); \
                return;                                                 \
            }                                                           \
            __value = (__type)__tmp_v;                                  \
        } while(0)

#endif
