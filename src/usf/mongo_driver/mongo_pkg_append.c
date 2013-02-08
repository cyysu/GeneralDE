#include "cpe/pal/pal_platform.h"
#include "cpe/dr/dr_bson.h"
#include "usf/mongo_driver/mongo_pkg.h"
#include "usf/mongo_driver/mongo_driver.h"
#include "mongo_internal_ops.h"

#define MONGO_REQUREST_CHECK_APPEND(__pkg, __len)                       \
    if ((__len) > (mongo_pkg_capacity(__pkg) - mongo_pkg_size(__pkg))) { \
        CPE_ERROR(                                                      \
            (__pkg)->m_driver->m_em,                                    \
            "mongo_pkg: append data len overflow, len=%d, size=%d, capacity=%d!", \
            (int)(__len), (int)mongo_pkg_size(__pkg), (int)mongo_pkg_capacity(__pkg)); \
        return -1;                                                      \
    }                                                                   \
    if ((__pkg)->m_cur_doc_pos < 0) {                                   \
        CPE_ERROR(                                                      \
            (__pkg)->m_driver->m_em,                                    \
            "mongo_pkg: append document not start!");                   \
        return -1;                                                      \
    }


#define MONGO_REQUEST_APPEND_POS(__pkg, __cur_len) (((char*)(pkg + 1)) + __cur_len)

static void mongo_pkg_append(mongo_pkg_t pkg, const void *data, int len) {
    size_t cur_len = mongo_pkg_size(pkg);
    memcpy(MONGO_REQUEST_APPEND_POS(pkg, cur_len), data, len);
    pkg->m_cur_doc_pos += len;
    mongo_pkg_set_size(pkg, cur_len + len);
}

static void mongo_pkg_append_byte(mongo_pkg_t pkg, char data) {
    size_t cur_len = mongo_pkg_size(pkg);
    *MONGO_REQUEST_APPEND_POS(pkg, cur_len) = data;
    pkg->m_cur_doc_pos += 1;
    mongo_pkg_set_size(pkg, cur_len + 1);
}

static void mongo_pkg_append_32(mongo_pkg_t pkg, const void * data) {
    size_t cur_len = mongo_pkg_size(pkg);
    CPE_COPY_HTON32(MONGO_REQUEST_APPEND_POS(pkg, cur_len), data);
    pkg->m_cur_doc_pos += 4;
    mongo_pkg_set_size(pkg, cur_len + 4);
}

static void mongo_pkg_append_64(mongo_pkg_t pkg, const void * data) {
    size_t cur_len = mongo_pkg_size(pkg);
    CPE_COPY_HTON64(MONGO_REQUEST_APPEND_POS(pkg, cur_len), data);
    pkg->m_cur_doc_pos += 8;
    mongo_pkg_set_size(pkg, cur_len + 8);
}

static int mongo_pkg_append_estart(mongo_pkg_t pkg, int type, const char *name, const int dataSize ) {
    const int len = strlen(name) + 1;

    MONGO_REQUREST_CHECK_APPEND(pkg, 1 + len + dataSize);

    mongo_pkg_append_byte(pkg, (char)type);
    mongo_pkg_append(pkg, name, len);

    return 0;
}

static int mongo_pkg_append_string_base(
    mongo_pkg_t pkg, const char *name,
    const char *value, int len, bson_type type)
{
    int32_t sl = len + 1;

    if (mongo_pkg_append_estart(pkg, type, name, 4 + sl) != 0) return -1;
    mongo_pkg_append_32(pkg, &sl);
    mongo_pkg_append(pkg, value, sl - 1);
    mongo_pkg_append(pkg, "\0" , 1);
    return 0;
}

int mongo_pkg_append_int32(mongo_pkg_t pkg, const char *name, const int32_t i) {
    if (mongo_pkg_append_estart(pkg, BSON_INT, name, 4) != 0) return -1;
    mongo_pkg_append_32(pkg, &i);
    return 0;
}

int mongo_pkg_append_int64(mongo_pkg_t pkg, const char *name, const int64_t i) {
    if (mongo_pkg_append_estart(pkg, BSON_LONG, name, 8) != 0) return -1;
    mongo_pkg_append_64(pkg, &i);
    return 0;
}

int mongo_pkg_append_double(mongo_pkg_t pkg, const char *name, const double d) {
    if (mongo_pkg_append_estart(pkg, BSON_DOUBLE, name, 8) != 0) return -1;
    mongo_pkg_append_64(pkg, &d);
    return 0;
}

int mongo_pkg_append_string(mongo_pkg_t pkg, const char *name, const char *str) {
    return mongo_pkg_append_string_base(pkg, name, str, strlen(str), BSON_STRING);
}

int mongo_pkg_append_string_n(mongo_pkg_t pkg, const char *name, const char *str, int len) {
    return mongo_pkg_append_string_base(pkg, name, str, len, BSON_STRING);
}

int mongo_pkg_append_symbol(mongo_pkg_t pkg, const char *name, const char *str) {
    return mongo_pkg_append_string_base(pkg, name, str, strlen(str), BSON_SYMBOL);
}

int mongo_pkg_append_symbol_n(mongo_pkg_t pkg, const char *name, const char *str, int len) {
    return mongo_pkg_append_string_base(pkg, name, str, len, BSON_SYMBOL);
}

int mongo_pkg_append_code(mongo_pkg_t pkg, const char *name, const char *str) {
    return mongo_pkg_append_string_base(pkg, name, str, strlen(str), BSON_CODE);
}

int mongo_pkg_append_code_n(mongo_pkg_t pkg, const char *name, const char *str, int len) {
    return mongo_pkg_append_string_base(pkg, name, str, len, BSON_CODE);
}

int mongo_pkg_append_binary(mongo_pkg_t pkg, const char *name, char type, const char *str, int i_len) {
    int32_t len = i_len;
    if (type == BSON_BIN_BINARY_OLD) {
        int32_t subtwolen = len + 4;
        if (mongo_pkg_append_estart(pkg, BSON_BINDATA, name, 4 + 1 + 4 + len) != 0) return -1;
        mongo_pkg_append_32(pkg, &subtwolen);
        mongo_pkg_append_byte(pkg, type);
        mongo_pkg_append_32(pkg, &len);
        mongo_pkg_append(pkg, str, len);
    }
    else {
        if (mongo_pkg_append_estart(pkg, BSON_BINDATA, name, 4 + 1 + len) != 0) return -1;
        mongo_pkg_append_32(pkg, &len);
        mongo_pkg_append_byte(pkg, type);
        mongo_pkg_append(pkg, str, len);
    }

    return 0;
}

int mongo_pkg_append_bool(mongo_pkg_t pkg, const char *name, const int v) {
    if (mongo_pkg_append_estart(pkg, BSON_BOOL, name, 1) != 0) return -1;
    mongo_pkg_append_byte(pkg, v != 0);
    return 0;
}

int mongo_pkg_append_null(mongo_pkg_t pkg, const char *name) {
    if (mongo_pkg_append_estart(pkg, BSON_NULL, name, 0) != 0) return -1;
    return 0;
}

int mongo_pkg_append_undefined(mongo_pkg_t pkg, const char *name) {
    if (mongo_pkg_append_estart(pkg, BSON_UNDEFINED, name, 0) != 0) return -1;
    return 0;
}

int mongo_pkg_append_regex(mongo_pkg_t pkg, const char *name, const char *pattern, const char *opts) {
    const int plen = strlen(pattern) + 1;
    const int olen = strlen(opts) + 1;
    
    if (mongo_pkg_append_estart(pkg, BSON_REGEX, name, plen + olen) != 0) return -1;
    mongo_pkg_append(pkg, pattern, plen);
    mongo_pkg_append(pkg, opts, olen);

    return 0;
}

int mongo_pkg_append_timestamp(mongo_pkg_t pkg, const char *name, int time, int increment) {
    if (mongo_pkg_append_estart(pkg, BSON_TIMESTAMP, name, 8) != 0) return -1;
    mongo_pkg_append_32(pkg, &increment);
    mongo_pkg_append_32(pkg, &time);
    return 0;
}

int mongo_pkg_append_date(mongo_pkg_t pkg, const char *name, int64_t millis) {
    if (mongo_pkg_append_estart(pkg, BSON_DATE, name, 8) != 0) return -1;
    mongo_pkg_append_64(pkg, &millis);
    return 0;
}

int mongo_pkg_append_time_t(mongo_pkg_t pkg, const char *name, time_t secs) {
    return mongo_pkg_append_date(pkg, name, (int64_t)secs * 1000);
}

static uint32_t zero = 0;

int mongo_pkg_append_start_object(mongo_pkg_t pkg, const char *name) {
    if (pkg->m_stackPos >= (sizeof(pkg->m_stack) / sizeof(pkg->m_stack[0]))) return -1;
    if (mongo_pkg_append_estart(pkg, BSON_OBJECT, name, 5) != 0) return -1;
    pkg->m_stack[pkg->m_stackPos++] = mongo_pkg_size(pkg);
    mongo_pkg_append_32(pkg, &zero);
    return 0;
}

int mongo_pkg_append_finish_object(mongo_pkg_t pkg) {
    char *start;
    int32_t i;
    if (pkg->m_stackPos <= 0) return -1;
    MONGO_REQUREST_CHECK_APPEND(pkg, 1);
    mongo_pkg_append_byte(pkg, 0);

    --pkg->m_stackPos;
    start = (char*)(pkg + 1) + pkg->m_stack[pkg->m_stackPos];
    i = mongo_pkg_size(pkg) - pkg->m_stack[pkg->m_stackPos];
    CPE_COPY_HTON32(start, &i);

    return 0;
}

int mongo_pkg_append_start_array(mongo_pkg_t pkg, const char *name) {
    if (pkg->m_stackPos >= (sizeof(pkg->m_stack) / sizeof(pkg->m_stack[0]))) return -1;
    if (mongo_pkg_append_estart(pkg, BSON_ARRAY, name, 5) != 0) return -1;
    pkg->m_stack[pkg->m_stackPos++] = mongo_pkg_size(pkg);
    mongo_pkg_append_32(pkg, &zero);
    return 0;
}

int mongo_pkg_append_finish_array(mongo_pkg_t pkg) {
    return mongo_pkg_append_finish_object(pkg);
}

int mongo_pkg_doc_append(mongo_pkg_t pkg, LPDRMETA meta, void const * data, size_t capacity) {
    char * buf;
    size_t output_capacity;
    uint32_t pkg_size;
    int write_size;

    if (pkg->m_cur_doc_start >= 0) {
        CPE_ERROR(
            pkg->m_driver->m_em, "%s: mongo_pkg_doc_append: pkg is already started",
            mongo_driver_name(pkg->m_driver));
        return -1;
    }

    pkg_size = mongo_pkg_size(pkg);
    output_capacity = mongo_pkg_capacity(pkg) - pkg_size;
    buf = (char*)(pkg + 1);

    write_size = dr_bson_write(buf + pkg_size, output_capacity, data, capacity, meta, pkg->m_driver->m_em);
    if (write_size < 0) {
        CPE_ERROR(
            pkg->m_driver->m_em, "%s: mongo_pkg_doc_append: bson write fail!",
            mongo_driver_name(pkg->m_driver));
        return -1;
    }

    if (write_size < MONGO_EMPTY_DOCUMENT_SIZE) {
        CPE_ERROR(
            pkg->m_driver->m_em, "%s: append document, write size(%d) too small!",
            mongo_driver_name(pkg->m_driver), write_size);
        return -1;
    }

    ++pkg->m_doc_count;

    mongo_pkg_set_size(pkg, pkg_size + write_size);

    return 0;
}
