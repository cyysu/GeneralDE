#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dp/dp_request.h"
#include "gd/app/app_log.h"
#include "gd/app/app_context.h"
#include "usf/mongo_driver/mongo_driver.h"
#include "usf/mongo_driver/mongo_pkg.h"
#include "mongo_internal_ops.h"

mongo_pkg_t
mongo_pkg_create(mongo_driver_t driver, size_t capacity) {
    dp_req_t dp_req;
    mongo_pkg_t pkg;

    dp_req = dp_req_create(
        gd_app_dp_mgr(driver->m_app),
        mongo_pkg_type_name,
        sizeof(struct mongo_pkg) + capacity);
    if (dp_req == NULL) return NULL;

    pkg = (mongo_pkg_t)dp_req_data(dp_req);

    pkg->m_driver = driver;
    pkg->m_dp_req = dp_req;

    mongo_pkg_init(pkg);

    return pkg;
}

void mongo_pkg_free(mongo_pkg_t req) {
    dp_req_free(req->m_dp_req);
}

dp_req_t mongo_pkg_to_dp_req(mongo_pkg_t req) {
    return req->m_dp_req;
}

mongo_pkg_t mongo_pkg_from_dp_req(dp_req_t req) {
    if (cpe_hs_cmp(dp_req_type_hs(req), mongo_pkg_type_name) != 0) return NULL;
    return (mongo_pkg_t)dp_req_data(req);
}

mongo_driver_t mongo_pkg_driver(mongo_pkg_t req) {
    return req->m_driver;
}

void mongo_pkg_init(mongo_pkg_t pkg) {
    pkg->m_stackPos = 0;
    pkg->m_ns[0] = 0;
    pkg->m_doc_count = -1;
    pkg->m_cur_doc_start = -1;
    pkg->m_cur_doc_pos = -1;
    bzero(&pkg->m_pro_head, sizeof(pkg->m_pro_head));
    bzero(&pkg->m_pro_data, sizeof(pkg->m_pro_data));
    mongo_pkg_set_size(pkg, 0);
}

mongo_db_op_t mongo_pkg_op(mongo_pkg_t pkg) {
    return pkg->m_pro_head.op;
}

void mongo_pkg_set_op(mongo_pkg_t pkg, mongo_db_op_t op) {
    pkg->m_pro_head.op = op;
    bzero(&pkg->m_pro_data, sizeof(pkg->m_pro_data));

    switch(op) {
    case mongo_db_op_query:
        pkg->m_pro_data.m_query.number_to_return = 10;
        break;
    case mongo_db_op_get_more:
        pkg->m_pro_data.m_get_more.number_to_return = 10;
        break;
    default:
        break;
    }
}

uint32_t mongo_pkg_id(mongo_pkg_t pkg) {
    return pkg->m_pro_head.id;
}

void mongo_pkg_set_id(mongo_pkg_t pkg, uint32_t id) {
    pkg->m_pro_head.id = id;
}

uint32_t mongo_pkg_response_to(mongo_pkg_t pkg) {
    return pkg->m_pro_head.response_to;
}

void mongo_pkg_set_response_to(mongo_pkg_t pkg, uint32_t id) {
    pkg->m_pro_head.response_to = id;
}

const char * mongo_pkg_ns(mongo_pkg_t pkg) {
    return pkg->m_ns;
}

void mongo_pkg_set_ns(mongo_pkg_t pkg, const char * ns) {
    strncpy(pkg->m_ns, ns, sizeof(pkg->m_ns));
}

void mongo_pkg_append_ns(mongo_pkg_t pkg, const char * ns) {
    size_t len = strlen(pkg->m_ns);
    strncpy(pkg->m_ns + len, ns, sizeof(pkg->m_ns) - len);
}

void * mongo_pkg_data(mongo_pkg_t pkg) {
    return (pkg + 1);
}

int mongo_pkg_doc_open(mongo_pkg_t pkg) {
    uint32_t new_size;

    if (pkg->m_cur_doc_start >= 0) {
        CPE_ERROR(
            pkg->m_driver->m_em, "%s: mongo_pkg_doc_start: pkg is already started",
            mongo_driver_name(pkg->m_driver));
        return -1;
    }

    assert(pkg->m_stackPos == 0);

    new_size = mongo_pkg_size(pkg) + 4;
    if (mongo_pkg_set_size(pkg, new_size) != 0) {
        CPE_ERROR(
            pkg->m_driver->m_em, "%s: mongo_pkg_doc_start: pkg size overflow, new_size=%d, capacity=%d",
            mongo_driver_name(pkg->m_driver), new_size, (int)mongo_pkg_capacity(pkg));
        return -1;
    }

    ++pkg->m_doc_count;

    pkg->m_cur_doc_start = new_size - 4;
    pkg->m_cur_doc_pos =  new_size;

    return 0;
}

int mongo_pkg_doc_close(mongo_pkg_t pkg) {
    uint32_t new_size;
    uint32_t doc_size;
    char * buf;

    if (pkg->m_cur_doc_start < 0) {
        CPE_ERROR(
            pkg->m_driver->m_em, "%s: mongo_pkg_doc_close: document is already closed",
            mongo_driver_name(pkg->m_driver));
        return -1;
    }

    if (pkg->m_stackPos != 0) {
        CPE_ERROR(
            pkg->m_driver->m_em, "%s: mongo_pkg_doc_close: document data is not closed",
            mongo_driver_name(pkg->m_driver));
        return -1;
    }

    new_size = mongo_pkg_size(pkg) + 1;
    if (mongo_pkg_set_size(pkg, new_size) != 0) {
        CPE_ERROR(
            pkg->m_driver->m_em, "%s: mongo_pkg_doc_close: pkg size overflow, new_size=%d, capacity=%d",
            mongo_driver_name(pkg->m_driver), new_size, (int)mongo_pkg_capacity(pkg));
        return -1;
    }

    buf = (char *)(pkg + 1);
    doc_size = pkg->m_cur_doc_pos - pkg->m_cur_doc_start + 1;
    CPE_COPY_HTON32(buf + pkg->m_cur_doc_start, &doc_size);
    buf[new_size - 1] = 0;

    pkg->m_cur_doc_pos = -1;
    pkg->m_cur_doc_start = -1;

    return 0;
}

int mongo_pkg_set_size(mongo_pkg_t pkg, uint32_t size) {
    if (dp_req_set_size(pkg->m_dp_req, sizeof(struct mongo_pkg) + size) != 0) {
        return -1;
    }

    return 0;
}

uint32_t mongo_pkg_size(mongo_pkg_t pkg) {
    return dp_req_size(pkg->m_dp_req) - sizeof(struct mongo_pkg);
}

size_t mongo_pkg_capacity(mongo_pkg_t pkg) {
    return dp_req_capacity(pkg->m_dp_req) - sizeof(struct mongo_pkg);
}

int mongo_pkg_it(bson_iterator * it, mongo_pkg_t pkg, int doc_idx) {
    if (doc_idx > 0) {
        struct mongo_doc_it doc_it;
        void * doc;
        mongo_pkg_doc_it(&doc_it, pkg);

        while(doc_idx > 0 && (doc = mongo_pkg_doc_it_next(&doc_it))) {
            --doc_idx;
        }

        if (doc_idx == 0) {
            bson_iterator_from_buffer(it, (const char *)doc);
            return 0;
        }
        else {
            return -1;
        }
    }
    else {
        bson_iterator_from_buffer(it, (const char *)mongo_pkg_data(pkg));
        return 0;
    }
}

int mongo_pkg_find(bson_iterator * it, mongo_pkg_t pkg, int doc_idx, const char * path) {
    return -1;
}

void mongo_pkg_doc_count_update(mongo_pkg_t pkg) {
    uint32_t doc_start = 0;
    uint32_t doc_size;
    uint32_t total_size = mongo_pkg_size(pkg);
    char * buf = (char*)(pkg + 1);

    pkg->m_doc_count = 0;

    while(doc_start + 4 < total_size) {
        ++pkg->m_doc_count;

        CPE_COPY_HTON32(&doc_size, buf + doc_start);
        doc_start += doc_size;
    }
}

int mongo_pkg_doc_count(mongo_pkg_t pkg) {
    return pkg->m_doc_count;
}

static void mongo_pkg_data_dump_i(write_stream_t stream, const void * data, int depth) {
    bson_iterator i;
    const char *key;
    bson_timestamp_t ts;
    char oidhex[25];
    bson scope;
    bson_iterator_from_buffer( &i, data);

    while(bson_iterator_next(&i)) {
        bson_type t = bson_iterator_type( &i);
        if (t == 0) break;

        key = bson_iterator_key( &i);

        stream_putc_count(stream,  ' ', depth << 2);

        switch (t) {
        case BSON_DOUBLE:
            stream_printf(stream,  "%s : %d(%s)\t%f" , key, t, "BSON_DOUBLE", bson_iterator_double( &i ));
            break;
        case BSON_STRING:
            stream_printf(stream,  "%s : %d(%s)\t%s" , key, t, "BSON_STRING", bson_iterator_string( &i ));
            break;
        case BSON_SYMBOL:
            stream_printf(stream,  "%s : %d(%s)\t%s" , key, t, "BSON_SYMBOL", bson_iterator_string( &i ));
            break;
        case BSON_OID:
            bson_oid_to_string( bson_iterator_oid( &i ), oidhex);
            stream_printf(stream,  "%s : %d(%s)\t%s" , key, t, "BSON_OID", oidhex);
            break;
        case BSON_BOOL:
            stream_printf(stream,  "%s : %d(%s)\t%s" , key, t, "BSON_BOOL", bson_iterator_bool( &i ) ? "true" : "false");
            break;
        case BSON_DATE:
            stream_printf(stream,  "%s : %d(%s)\t%ld", key, t, "BSON_DATE", (long int)bson_iterator_date( &i ));
            break;
        case BSON_BINDATA:
            stream_printf(stream,  "%s : %d(%s)", key, t, "BSON_BINDATA");
            break;
        case BSON_UNDEFINED:
            stream_printf(stream,  "%s : %d(%s)", key, t, "BSON_UNDEFINED");
            break;
        case BSON_NULL:
            stream_printf(stream,  "%s : %d(%s)", key, t, "BSON_NULL");
            break;
        case BSON_REGEX:
            stream_printf(stream,  "%s : %d(%s)\t: %s", key, t, "BSON_REGEX", bson_iterator_regex( &i ));
            break;
        case BSON_CODE:
            stream_printf(stream,  "%s : %d(%s)\t: %s", key, t, "BSON_CODE", bson_iterator_code( &i ));
            break;
        case BSON_CODEWSCOPE:
            stream_printf(stream,  "%s : %d(%s)\t: %s", key, t, "BSON_CODE_W_SCOPE", bson_iterator_code( &i ));
            bson_init( &scope);
            bson_iterator_code_scope( &i, &scope);
            stream_printf(stream,  "\n\t SCOPE: ");
            bson_print( &scope);
            break;
        case BSON_INT:
            stream_printf(stream,  "%s : %d(%s)\t%d" , key, t, "BSON_INT", bson_iterator_int( &i ));
            break;
        case BSON_LONG:
            stream_printf(stream,  "%s : %d(%s)\t%lld", key, t, "BSON_LONG", (uint64_t)bson_iterator_long( &i ));
            break;
        case BSON_TIMESTAMP:
            ts = bson_iterator_timestamp( &i);
            stream_printf(stream,  "%s : %d(%s)\ti: %d, t: %d", key, t, "BSON_TIMESTAMP", ts.i, ts.t);
            break;
        case BSON_OBJECT:
            stream_printf(stream,  "%s : %d(%s)\n", key, t, "BSON_OBJECT");
            mongo_pkg_data_dump_i(stream, bson_iterator_value(&i), depth + 1);
            break;
        case BSON_ARRAY:
            stream_printf(stream,  "%s : %d(%s)\n", key, t, "BSON_ARRAY");
            mongo_pkg_data_dump_i(stream, bson_iterator_value(&i), depth + 1);
            break;
        default:
            stream_printf(stream, "%s : %d(%s)" , key, t, "???");
        }
        stream_printf(stream, "\n");
    }
}

const char * mongo_pkg_dump(mongo_pkg_t req, mem_buffer_t buffer, int level) {
    struct write_stream_buffer stream;
    struct mongo_doc_it doc_it;
    mongo_doc_t doc;
    int i = 0;

    mem_buffer_clear_data(buffer);

    write_stream_buffer_init(&stream, buffer);

    stream_putc_count((write_stream_t)&stream,  ' ', level << 2);
    stream_printf((write_stream_t)&stream, "*********** head **********\n", i);
    stream_putc_count((write_stream_t)&stream,  ' ', (level + 1) << 2);
    stream_printf(
        (write_stream_t)&stream, "id=%d, response_to=%d, ",
        req->m_pro_head.id, req->m_pro_head.response_to);

    switch(req->m_pro_head.op) {
    case mongo_db_op_query:
        stream_printf(
            (write_stream_t)&stream, "op=query, ns=%s, flags=%d, number_to_skip=%d, number_to_return=%d",
            req->m_ns, req->m_pro_data.m_query.flags, req->m_pro_data.m_query.number_to_skip, req->m_pro_data.m_query.number_to_return);
        break;
    case mongo_db_op_get_more:
        stream_printf(
            (write_stream_t)&stream, "op=get more, ns=%s, number_to_return=%d, cursor_id=%llu",
            req->m_ns, req->m_pro_data.m_get_more.number_to_return, req->m_pro_data.m_get_more.cursor_id);
        break;
    case mongo_db_op_insert:
        stream_printf((write_stream_t)&stream, "op=insert, ns=%s", req->m_ns);
        break;
    case mongo_db_op_replay:
        stream_printf((write_stream_t)&stream, "op=replay, response_flag=(");
        if (req->m_pro_data.m_reply.response_flag & mongo_pro_flags_reply_cursor_not_found) stream_printf((write_stream_t)&stream, " cursor_not_found");
        if (req->m_pro_data.m_reply.response_flag & mongo_pro_flags_reply_query_fail) stream_printf((write_stream_t)&stream, " query_fail");
        if (req->m_pro_data.m_reply.response_flag & mongo_pro_flags_reply_shared_config_state) stream_printf((write_stream_t)&stream, " shared_config_state");
        if (req->m_pro_data.m_reply.response_flag & mongo_pro_flags_reply_await_capable) stream_printf((write_stream_t)&stream, " await_capable");
        stream_printf(
            (write_stream_t)&stream, "), cursor_id=%llu, starting_from=%d, number_retuned=%d",
            req->m_pro_data.m_reply.cursor_id, req->m_pro_data.m_reply.starting_from, req->m_pro_data.m_reply.number_retuned);
        break;
    case mongo_db_op_msg:
        stream_printf((write_stream_t)&stream, "op=msg");
        break;
    case mongo_db_op_kill_cursors:
        stream_printf((write_stream_t)&stream, "op=kill cursors, cursor_count=%d", req->m_pro_data.m_kill_cursor.cursor_count);
        break;
    case mongo_db_op_update:
        stream_printf((write_stream_t)&stream, "op=update, ns=%s, flags=%d", req->m_ns, req->m_pro_data.m_update.flags);
        break;
    case mongo_db_op_delete:
        stream_printf((write_stream_t)&stream, "op=delete, ns=%s, flags=%d", req->m_ns, req->m_pro_data.m_delete.flags);
        break;
    default:
        stream_printf((write_stream_t)&stream, "op=%d, unknown!", req->m_pro_head.op);
        break;
    }
    stream_printf((write_stream_t)&stream, "\n");

    i = 0;
    mongo_pkg_doc_it(&doc_it, req);
    while((doc = mongo_pkg_doc_it_next(&doc_it))) {

        stream_putc_count((write_stream_t)&stream,  ' ', level << 2);
        stream_printf((write_stream_t)&stream, "*********** doc %d **********\n", i);

        mongo_pkg_data_dump_i((write_stream_t)&stream, mongo_doc_data(doc), level + 1);

        ++i;
    }

    stream_putc((write_stream_t)&stream, 0);

    return mem_buffer_make_continuous(buffer, 0);
}

int mongo_pkg_validate(mongo_pkg_t pkg, error_monitor_t em) {
    if (pkg->m_cur_doc_start != -1) {
        CPE_ERROR(
            em, "%s: pkg_validate: pkg document not closed",
            mongo_driver_name(pkg->m_driver));
        return -1;
    }

    return 0;
}

struct mongo_pkg_doc_it_data {
    mongo_pkg_t m_pkg;
    int32_t m_pos;
};

static mongo_doc_t mongo_pkg_doc_it_do_next(struct mongo_doc_it * it) {
    struct mongo_pkg_doc_it_data * data = (struct mongo_pkg_doc_it_data *)&it->m_data;
    uint32_t total_size;
    uint32_t doc_size;
    char * buf;
    void * r;

    if (data->m_pkg == NULL) return NULL;

    total_size =  mongo_pkg_size(data->m_pkg);
    buf = (char*)(data->m_pkg + 1);

    if (data->m_pos < 0 || data->m_pos + 4 > total_size) return NULL;
    CPE_COPY_HTON32(&doc_size, buf + data->m_pos);

    if (doc_size < MONGO_EMPTY_DOCUMENT_SIZE) return NULL;

    if (data->m_pos + doc_size > total_size) return NULL;

    r = buf + data->m_pos;

    data->m_pos += doc_size;

    return r;
}

void mongo_pkg_doc_it(mongo_doc_it_t it, mongo_pkg_t pkg) {
    struct mongo_pkg_doc_it_data * data = (struct mongo_pkg_doc_it_data *)&it->m_data;

    assert(sizeof(*data) <= sizeof(it->m_data));
    data->m_pkg = pkg;
    data->m_pos = 0;
    it->next = mongo_pkg_doc_it_do_next;
}

int32_t mongo_doc_size(mongo_doc_t doc) {
    int32_t doc_size;
    CPE_COPY_HTON32(&doc_size, doc);
    return doc_size;
}

void * mongo_doc_data(mongo_doc_t doc) {
    return (void*)doc;
}

void mongo_pkg_cmd_init(mongo_pkg_t pkg, const char * ns) {
    const char * sep;

    mongo_pkg_init(pkg);
    mongo_pkg_set_op(pkg, mongo_db_op_query);

    mongo_pkg_set_ns(pkg, ns);

    sep = strchr(pkg->m_ns, '.');
    if (sep) {
        size_t len = sep - pkg->m_ns;
        strncpy(sep, ".$cmd", sizeof(pkg->m_ns) - len);
    }
    else {
        mongo_pkg_append_ns(pkg, ".$cmd");
    }

    mongo_pkg_query_set_number_to_return(pkg, 1);
}

int32_t mongo_pkg_query_flags(mongo_pkg_t pkg) {
    assert(pkg->m_pro_head.op == mongo_db_op_query);
    return pkg->m_pro_data.m_query.flags;
}

void mongo_pkg_query_set_flag(mongo_pkg_t pkg, mongo_pro_flags_query_t flag) {
    assert(pkg->m_pro_head.op == mongo_db_op_query);
    pkg->m_pro_data.m_query.flags &= (uint32_t)flag;
}

void mongo_pkg_query_unset_flag(mongo_pkg_t pkg, mongo_pro_flags_query_t flag) {
    assert(pkg->m_pro_head.op == mongo_db_op_query);
    pkg->m_pro_data.m_query.flags |= ~ ((uint32_t)flag);
}

int32_t mongo_pkg_query_number_to_skip(mongo_pkg_t pkg) {
    assert(pkg->m_pro_head.op == mongo_db_op_query);
    return pkg->m_pro_data.m_query.number_to_skip;
}

void mongo_pkg_query_set_number_to_skip(mongo_pkg_t pkg, int32_t number_to_skip) {
    assert(pkg->m_pro_head.op == mongo_db_op_query);
    pkg->m_pro_data.m_query.number_to_skip = number_to_skip;
}

int32_t mongo_pkg_query_number_to_return(mongo_pkg_t pkg) {
    assert(pkg->m_pro_head.op == mongo_db_op_query);
    return pkg->m_pro_data.m_query.number_to_return;
}

void mongo_pkg_query_set_number_to_return(mongo_pkg_t pkg, int32_t number_to_return) {
    assert(pkg->m_pro_head.op == mongo_db_op_query);
    pkg->m_pro_data.m_query.number_to_return = number_to_return;
}

CPE_HS_DEF_VAR(mongo_pkg_type_name, "mongo_pkg_type");

