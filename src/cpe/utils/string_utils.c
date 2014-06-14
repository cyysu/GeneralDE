#include <assert.h>
#include <ctype.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/string_utils.h"

char *
cpe_str_dup_range(char * buf, size_t capacity, const char * begin, const char * end) {
    int size;

    if (buf == NULL) return NULL;

    size = end - begin;
    if (size < 0 || ((size_t)size) + 1 > capacity) return NULL;

    memcpy(buf, begin, size);
    buf[size] = 0;

    return buf;
}

char *
cpe_str_dup_len(char * buf, size_t capacity, const char * begin, size_t size) {
    if (buf == NULL) return NULL;

    if (size + 1 > capacity) return NULL;

    memcpy(buf, begin, size);
    buf[size] = 0;

    return buf;
}

char * cpe_str_mem_dup(mem_allocrator_t alloc, const char * str) {
    size_t capacity = strlen(str) + 1;
    char * buf = mem_alloc(alloc, capacity);
    if (buf == NULL) return NULL;
    memcpy(buf, str, capacity);
    return buf;
}

int cpe_str_buf_is_overflow(cpe_str_buf_t buf) {
    return buf->m_overflow;
}

int cpe_str_buf_append(cpe_str_buf_t buf, const char * data, size_t data_len) {
    assert(buf);
    assert(data);
    assert(buf->m_buf);

    if (data_len + buf->m_size + 1 > buf->m_capacity) {
        data_len = buf->m_capacity - buf->m_size - 1;
        buf->m_overflow = 1;
    }

    if (data_len) {
        memcpy(buf->m_buf + buf->m_size, data, data_len + 1);
        buf->m_size += data_len;
    }

    buf->m_buf[buf->m_size] = 0;

    return buf->m_overflow;
}

int cpe_str_buf_cat(cpe_str_buf_t buf, const char * data) {
    return cpe_str_buf_append(buf, data, strlen(data));
}

int cpe_str_buf_cpy(cpe_str_buf_t buf, const char * data) {
    size_t data_len;

    assert(buf);
    assert(data);
    assert(buf->m_buf);

    data_len = strlen(data);

    if (data_len + 1 > buf->m_capacity) {
        data_len = buf->m_capacity - 1;
        buf->m_overflow = 1;
    }
    else {
        buf->m_overflow = 0;
    }

    if (data_len) {
        memcpy(buf->m_buf, data, data_len + 1);
        buf->m_size = data_len;
    }

    return buf->m_overflow;
}

int cpe_str_buf_cat_printf(cpe_str_buf_t buf, const char * fmt, ...) {
    va_list args;
    size_t capacity;
    int r;

    assert(buf);
    assert(fmt);
    assert(buf->m_buf);

    capacity = buf->m_capacity - buf->m_size;

    va_start(args, fmt);
    r = vsnprintf(buf->m_buf + buf->m_size, capacity, fmt, args);

    if (r < 0) {
        buf->m_overflow = 1;
    }
    else {
        buf->m_size += r;
    }

    va_end(args);

    return buf->m_overflow;
}

int cpe_str_buf_printf(cpe_str_buf_t buf, const char * fmt, ...) {
    va_list args;
    int r;

    assert(buf);
    assert(fmt);
    assert(buf->m_buf);

    va_start(args, fmt);
    r = vsnprintf(buf->m_buf, buf->m_capacity, fmt, args);

    if (r < 0) {
        buf->m_overflow = 1;
    }
    else {
        buf->m_size += r;
        buf->m_overflow = 0;
    }

    va_end(args);

    return buf->m_overflow;
}

void cpe_str_toupper(char * data) {
    while(*data) {
        *data = toupper(*data);
        ++data;
    }
}

void cpe_str_tolower(char * data) {
    while(*data) {
        *data = tolower(*data);
        ++data;
    }
}

int cpe_str_cmp_part(const char * part_str, size_t part_str_len, const char * full_str) {
    int r = strncmp(part_str, full_str, part_str_len);
    if (r == 0) return full_str[part_str_len] == 0 ? 0 : - (int)part_str_len;
    return r;
}

uint64_t cpe_str_parse_byte_size_with_dft(const char * astring, uint64_t dft) {
    uint64_t r;
    if (cpe_str_parse_byte_size(&r, astring) != 0) return dft;
    return r;
}

int cpe_str_parse_byte_size(uint64_t * result, const char * astring) {
    size_t sz;
    char * last = NULL;
    long res;
    size_t numsize;

    if (astring == NULL) return -1;

    sz = strlen (astring);
    res = strtol(astring, &last, 10);
    if (res <= 0) return -1;

    assert(last);
    numsize  = last - astring;

    if (numsize == sz) {
        if (result) *result = res;
        return 0;
    }

    if (numsize + 2 != sz) return -1;

    if (astring[sz - 1] == 'B' || astring[sz - 1] == 'b') {
        switch (astring[ sz - 2 ]) {
	    case 'K':
	    case 'k':
            res *= 1024;
            break;
	    case 'M':
	    case 'm':
            res *= 1024 * 1024;
            break;
	    case 'G':
	    case 'g':
            res *= 1024 * 1024 * 1024;
            break;
	    default:
            return -1;
        }
    }

    if (result) *result = res;
    return 0;
}

uint64_t cpe_str_parse_timespan_ms_with_dft(const char * astring, uint64_t dft) {
    uint64_t r;
    if (cpe_str_parse_timespan_ms(&r, astring) != 0) return dft;
    return r;
}

int cpe_str_parse_timespan_ms(uint64_t * result, const char * astring) {
    size_t sz;
    char * last = NULL;
    long res;
    size_t numsize;
    size_t post_size;
    const char * post_str;

    if (astring == NULL) return -1;

    sz = strlen (astring);
    res = strtol(astring, &last, 10);
    if (res <= 0) return -1;

    assert(last);
    numsize  = last - astring;
    post_size = sz - numsize;

    post_str = astring + numsize;
    if (post_size == 1 && post_str[0] == 's') {
        res *= 1000;
    }
    else if (post_size == 1 && post_str[0] == 'm') {
        res *= 60 * 1000;
    }
    else if (post_size == 2 && post_str[0] == 'm' && post_str[1] == 's') {
    }
    else if (post_size == 1 && post_str[0] == 'h') {
        res *= 60 * 60 * 1000;
    }
    else {
        return -1;
    }

    if (result) *result = res;
    return 0;
}

char * cpe_str_trim_head(char * p) {
    char v;
    while((v = *p) != 0) {
        if (v == ' ' || v == '\t' || v == '\r' || v == '\n') {
            ++p;
        }
        else {
            return p;
        }
    }

    return p;
}

char * cpe_str_trim_tail(char * p, const char * head) {
    while((p - 1) >= head) {
        char v = *(p - 1);

        if (v == ' ' || v == '\t' || v == '\r' || v == '\n') {
            --p;
        }
        else {
            return p;
        }
        
    }

    return p;
}

char * cpe_str_mask_uint16(uint16_t v, char * buf, size_t buf_size) {
    uint8_t i;
    uint8_t bit_count = sizeof(v) * 8;
    assert(buf);
    assert(buf_size >= bit_count + 1);

    for(i = 0; i < bit_count; ++i) {
        buf[i] = (v & 1 << (bit_count - i - 1)) ? '1' : '0';
    }

    buf[bit_count] = 0;

    return buf;
}
