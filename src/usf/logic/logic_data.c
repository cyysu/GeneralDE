#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_data.h"
#include "usf/logic/logic_data.h"
#include "logic_internal_ops.h"

logic_data_t
logic_data_get_or_create(logic_context_t context, LPDRMETA meta, size_t capacity) {
    const char * name;
    logic_data_t old_data;
    logic_data_t new_data;

    if (capacity == 0) capacity = dr_meta_size(meta);

    name = dr_meta_name(meta);
    old_data = logic_data_find(context, name);
    if (old_data && old_data->m_capacity >= capacity) return old_data;

    new_data = (logic_data_t)mem_alloc(context->m_mgr->m_alloc, sizeof(struct logic_data) + capacity);
    if (new_data == NULL) return NULL;

    new_data->m_ctx = context;
    new_data->m_name = name;
    new_data->m_meta = meta;
    new_data->m_capacity = capacity;
    cpe_hash_entry_init(&new_data->m_hh);

    if (old_data) {
        memcpy(new_data + 1, old_data + 1, old_data->m_capacity);
        logic_data_free(old_data);
    }
    else {
        bzero(new_data + 1, capacity);
        dr_meta_set_defaults(new_data + 1, capacity, meta, 0);
    }

    if (cpe_hash_table_insert_unique(&context->m_mgr->m_datas, new_data) != 0) {
        mem_free(context->m_mgr->m_alloc, new_data);
        return NULL;
    }

    TAILQ_INSERT_TAIL(&context->m_datas, new_data, m_next);

    return new_data;
}

void logic_data_free(logic_data_t data) {
    assert(data);
    cpe_hash_table_remove_by_ins(&data->m_ctx->m_mgr->m_datas, data);
    TAILQ_REMOVE(&data->m_ctx->m_datas, data, m_next);
    mem_free(data->m_ctx->m_mgr->m_alloc, data);
}

void logic_data_free_all(logic_manage_t mgr) {
    struct cpe_hash_it data_it;
    logic_data_t data;

    cpe_hash_it_init(&data_it, &mgr->m_datas);

    data = cpe_hash_it_next(&data_it);
    while (data) {
        logic_data_t next = cpe_hash_it_next(&data_it);
        logic_data_free(data);
        data = next;
    }
}

logic_data_t logic_data_find(logic_context_t context, const char * name) {
    struct logic_data key;

    key.m_ctx = context;
    key.m_name = name;

    return (logic_data_t)cpe_hash_table_find(&context->m_mgr->m_datas, &key);
}

LPDRMETA logic_data_meta(logic_data_t data) {
    return data->m_meta;
}

void * logic_data_data(logic_data_t data) {
    return data + 1;
}

size_t logic_data_capacity(logic_data_t data) {
    return data->m_capacity;
}

const char * logic_data_name(logic_data_t data) {
    return data->m_name;
}

uint32_t logic_data_hash(const struct logic_data * data) {
    return (cpe_hash_str(data->m_name, strlen(data->m_name)) << 16)
        | (data->m_ctx->m_id & 0xFF);
}

int logic_data_cmp(const struct logic_data * l, const struct logic_data * r) {
    return l->m_ctx->m_id == r->m_ctx->m_id
        && strcmp(l->m_name, r->m_name) == 0;
}

#define LOGIC_DATA_DEF_DATA_NO_TRY_FUN(__to, __to_type)                 \
    __to_type logic_data_read_ ## __to(                                 \
        logic_context_t context,                                        \
        const char * path)                                              \
    {                                                                   \
        char name[128];                                                 \
        const char * sep;                                               \
        const char * sub_path;                                          \
        size_t name_len;                                                \
        logic_data_t data;                                              \
                                                                        \
        if (path == NULL) return (__to_type)0;                          \
                                                                        \
        sep = strchr(path, '.');                                        \
        if (sep == NULL) return (__to_type)0;                           \
                                                                        \
        name_len = sep - path;                                          \
        if ((name_len + 1) > sizeof(name)) return (__to_type)0;         \
                                                                        \
        memcpy(name, path, name_len);                                   \
        name[name_len] = 0;                                             \
                                                                        \
        sub_path = sep + 1;                                             \
                                                                        \
        data = logic_data_find(context, name);                          \
        if (data == NULL) return (__to_type)0;                          \
                                                                        \
        return dr_meta_read_ ## __to(logic_data_data(data), data->m_meta, sub_path); \
    }                                                                   \
    __to_type logic_data_read_with_dft_ ## __to(                       \
        logic_context_t context,                                        \
        const char * path,                                              \
        __to_type dft)                                                  \
    {                                                                   \
        char name[128];                                                 \
        const char * sep;                                               \
        const char * sub_path;                                          \
        size_t name_len;                                                \
        logic_data_t data;                                              \
                                                                        \
        if (path == NULL) return dft;                                   \
                                                                        \
        sep = strchr(path, '.');                                        \
        if (sep == NULL) return dft;                                    \
                                                                        \
        name_len = sep - path;                                          \
        if ((name_len + 1) > sizeof(name)) return dft;                  \
                                                                        \
        memcpy(name, path, name_len);                                   \
        name[name_len] = 0;                                             \
                                                                        \
        sub_path = sep + 1;                                             \
                                                                        \
        data = logic_data_find(context, name);                          \
        if (data == NULL) return dft;                                   \
                                                                        \
        return dr_meta_read_with_dft_ ## __to(logic_data_data(data), data->m_meta, sub_path, dft); \
    }

#define LOGIC_DATA_DEF_DATA_FUN(__to, __to_type)                        \
    int logic_data_try_read_ ## __to(                                   \
        __to_type * result,                                             \
        logic_context_t context,                                        \
        const char * path, error_monitor_t em)                          \
    {                                                                   \
        char name[128];                                                 \
        const char * sep;                                               \
        const char * sub_path;                                          \
        size_t name_len;                                                \
        logic_data_t data;                                              \
        int r;                                                          \
                                                                        \
        if (path == NULL) {                                             \
            CPE_ERROR(em, "logic_data_read_data: path is null!");       \
            return -1;                                                  \
        }                                                               \
                                                                        \
        sep = strchr(path, '.');                                        \
        if (sep == NULL) {                                              \
            CPE_ERROR(em, "logic_data_read_data: can`t find first name from path(%s)!", path); \
            return -1;                                                  \
        }                                                               \
                                                                        \
        name_len = sep - path;                                          \
        if ((name_len + 1) > sizeof(name)) {                            \
            CPE_ERROR(em, "logic_data_read_data: first name too long, path(%s)!", path); \
            return -1;                                                  \
        }                                                               \
                                                                        \
        memcpy(name, path, name_len);                                   \
        name[name_len] = 0;                                             \
                                                                        \
        sub_path = sep + 1;                                             \
                                                                        \
        data = logic_data_find(context, name);                          \
        if (data == NULL) {                                             \
            CPE_ERROR(em, "logic_data_read_data: data %s not exist!", name); \
            return -1;                                                  \
        }                                                               \
                                                                        \
        r = dr_meta_try_read_ ## __to(result, logic_data_data(data), data->m_meta, sub_path, em); \
        if (r != 0) {                                                   \
            CPE_ERROR(em, "logic_data_read_data: read %s from %s fail!", sub_path, name); \
        }                                                               \
        return r;                                                       \
    }                                                                   \
    LOGIC_DATA_DEF_DATA_NO_TRY_FUN(__to, __to_type)

LOGIC_DATA_DEF_DATA_FUN(int8, int8_t);
LOGIC_DATA_DEF_DATA_FUN(uint8, uint8_t);
LOGIC_DATA_DEF_DATA_FUN(int16, int16_t);
LOGIC_DATA_DEF_DATA_FUN(uint16, uint16_t);
LOGIC_DATA_DEF_DATA_FUN(int32, int32_t);
LOGIC_DATA_DEF_DATA_FUN(uint32, uint32_t);
LOGIC_DATA_DEF_DATA_FUN(int64, int64_t);
LOGIC_DATA_DEF_DATA_FUN(uint64, uint64_t);
LOGIC_DATA_DEF_DATA_FUN(float, float);
LOGIC_DATA_DEF_DATA_FUN(double, double);
LOGIC_DATA_DEF_DATA_NO_TRY_FUN(string, const char *);
