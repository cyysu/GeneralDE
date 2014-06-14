#include <errno.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/file.h"
#include "cpe/utils/stream_buffer.h"
#include "ui/model/ui_data_src.h"
#include "ui/model/ui_data_mgr.h"
#include "ui_np_load_utils.h"
#include "ui_np_utils.h"

char * ui_data_np_load_src_to_buff(mem_buffer_t data_buff, ui_data_src_t src, error_monitor_t em) {
    struct mem_buffer path_buff;
    struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(&path_buff);
    char * path;
    char * r;

    mem_buffer_init(&path_buff, NULL);

    ui_data_src_path_print_to((write_stream_t)&stream, src, NULL);
    stream_printf((write_stream_t)&stream, ".%s", ui_data_np_postfix(ui_data_src_type(src)));
    stream_putc((write_stream_t)&stream, 0);
    path = mem_buffer_make_continuous(&path_buff, 0);

    if (file_load_to_buffer(data_buff, path, NULL) < 0 || mem_buffer_append_char(data_buff, 0) <= 0) {
        CPE_ERROR(em, "read file %s fail, error=%d (%s)!", path, errno, strerror(errno));
        mem_buffer_clear(&path_buff);
        return NULL;
    }
    mem_buffer_clear(&path_buff);

    r = mem_buffer_make_continuous(data_buff, 0);
    if (r == NULL) {
        CPE_ERROR(em, "load src to buff: no memory!");
        return NULL;
    }

    return r;
}

int ui_data_np_find_attr_long(long * result, const char * attr_name, int nb_attributes, const void * attributes, error_monitor_t em) {
    int index, indexAttribute;

    for(index = 0, indexAttribute = 0;
        indexAttribute < nb_attributes;
        ++indexAttribute, index += 5)
    {
        const char *localname = ((const char * *)attributes)[index];
        const char *valueBegin;
        const char *valueEnd;
        char * endp;

        if (strcmp(localname, attr_name) != 0) continue;

        valueBegin = ((const char * *)attributes)[index+3];
        valueEnd = ((const char * *)attributes)[index+4];

        *result = strtol(valueBegin, &endp, 10);
        if (endp != valueEnd) {
            char buf[64];
            size_t len = valueEnd - valueBegin;
            if (len >= sizeof(buf)) len = sizeof(buf) - 1;
            memcpy(buf, valueBegin, len);
            buf[len] = 0;

            CPE_ERROR(em, "read attr %s from value %s fail!", attr_name, buf);
            return -1; 
        }

        return 0;
    }

    CPE_ERROR(em, "read attr %s not exist!", attr_name);
    return -1;
}

int ui_data_np_find_attr_float(float * result, const char * attr_name, int nb_attributes, const void * attributes, error_monitor_t em) {
    int index, indexAttribute;

    for(index = 0, indexAttribute = 0;
        indexAttribute < nb_attributes;
        ++indexAttribute, index += 5)
    {
        const char *localname = ((const char * *)attributes)[index];
        const char *valueBegin;
        const char *valueEnd;
        char * endptr;

        if (strcmp(localname, attr_name) != 0) continue;

        valueBegin = ((const char * *)attributes)[index+3];
        valueEnd = ((const char * *)attributes)[index+4];

        *result = strtof(((const char * *)attributes)[index+3], &endptr);
        if (endptr != valueEnd) {
            char buf[64];
            size_t len = valueEnd - valueBegin;
            if (len >= sizeof(buf)) len = sizeof(buf) - 1;
            memcpy(buf, valueBegin, len);
            buf[len] = 0;
            CPE_ERROR(em, "read attr %s from value %s fail!", attr_name, buf);
            return -1; 
        }

        return 0;
    }

    CPE_ERROR(em, "read attr %s not exist!", attr_name);
    return -1;
}

int ui_data_np_find_attr_bool(uint8_t * result, const char * attr_name, int nb_attributes, const void * attributes, error_monitor_t em) {
    char _f[] = "False";
    char _t[] = "True";
    int index, indexAttribute;

    for(index = 0, indexAttribute = 0;
        indexAttribute < nb_attributes;
        ++indexAttribute, index += 5)
    {
        const char *localname = ((const char * *)attributes)[index];
        const char *valueBegin;
        const char *valueEnd;
        size_t data_len;

        if (strcmp(localname, attr_name) != 0) continue;

        valueBegin = ((const char * *)attributes)[index+3];
        valueEnd = ((const char * *)attributes)[index+4];
        data_len = valueEnd - valueBegin;

        if (data_len + 1 == sizeof(_f) && memcmp(valueBegin, _f, data_len) == 0) {
            *result = 0;
            return 0;
        }
        else if (data_len + 1 == sizeof(_t) && memcmp(valueBegin, _t, data_len) == 0) {
            *result = 1;
            return 0;
        }
        else {
            char buf[64];
            if (data_len >= sizeof(buf)) data_len = sizeof(buf) - 1;
            memcpy(buf, valueBegin, data_len);
            buf[data_len] = 0;
            CPE_ERROR(em, "read attr %s from value %s fail!", attr_name, buf);
            return -1; 
        }
    }

    CPE_ERROR(em, "read attr %s not exist!", attr_name);
    return -1;
}

int ui_data_np_read_value_bool(uint8_t * result, const char * data, size_t data_len) {
    char _f[] = "False";
    char _t[] = "True";

    if (data_len + 1 == sizeof(_f) && memcmp(data, _f, data_len) == 0) {
        *result = 0;
        return 0;
    }
    else if (data_len + 1 == sizeof(_t) && memcmp(data, _t, data_len) == 0) {
        *result = 1;
        return 0;
    }
    else {
        return -1;
    }
}

int ui_data_np_read_value_long(long * result, const char * data, size_t data_len) {
    char * endptr;
    *result = strtol(data, &endptr, 10);
    if (endptr - data != data_len) {
        return -1;
    }
    return 0;
}

int ui_data_np_read_value_float(float * result, const char * data, size_t data_len) {
    char * endptr;
    *result = strtof(data, &endptr);
    if (endptr - data != data_len) return -1;
    return 0;
}

const char * ui_data_np_find_attr_string(
    char * buff, size_t capacity, const char * attr_name, int nb_attributes, const void * attributes, error_monitor_t em)
{
    int index, indexAttribute;

    for(index = 0, indexAttribute = 0;
        indexAttribute < nb_attributes;
        ++indexAttribute, index += 5)
    {
        const char *localname = ((const char * *)attributes)[index];

        if (strcmp(localname, attr_name) == 0) {
            const char *valueBegin = ((const char * *)attributes)[index+3];
            const char *valueEnd = ((const char * *)attributes)[index+4];
            size_t len = valueEnd - valueBegin;

            if ((len + 1) > capacity) len = capacity - 1;

            memcpy(buff, valueBegin, len);
            buff[len] = 0;
            return buff;
        }
    }

    CPE_ERROR(em, "read attr %s not exist!", attr_name);
    return NULL;
}
