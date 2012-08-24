#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dp/dp_request.h"
#include "gd/app/app_log.h"
#include "gd/app/app_context.h"
#include "usf/mongo_driver/mongo_driver.h"
#include "usf/mongo_driver/mongo_request.h"
#include "mongo_internal_ops.h"

mongo_request_t
mongo_request_create(mongo_driver_t driver, size_t capacity) {
    dp_req_t dp_req;
    mongo_request_t pkg;

    dp_req = dp_req_create(
        gd_app_dp_mgr(driver->m_app),
        mongo_request_type_name,
        sizeof(struct mongo_request) + capacity);
    if (dp_req == NULL) return NULL;

    pkg = (mongo_request_t)dp_req_data(dp_req);

    pkg->m_driver = driver;
    pkg->m_dp_req = dp_req;

    mongo_request_init(pkg);

    return pkg;
}

void mongo_request_free(mongo_request_t req) {
    dp_req_free(req->m_dp_req);
}

dp_req_t mongo_request_to_dp_req(mongo_request_t req) {
    return req->m_dp_req;
}

mongo_request_t mongo_request_from_dp_req(dp_req_t req) {
    if (cpe_hs_cmp(dp_req_type_hs(req), mongo_request_type_name) != 0) return NULL;
    return (mongo_request_t)dp_req_data(req);
}


mongo_driver_t mongo_request_driver(mongo_request_t req) {
    return req->m_driver;
}

void mongo_request_init(mongo_request_t pkg) {
    pkg->m_finished = 0;
    pkg->m_stackPos = 0;
    bzero(&pkg->m_pro_head, sizeof(pkg->m_pro_head));
    dp_req_set_size(pkg->m_dp_req, sizeof(struct mongo_request));
}

uint32_t mongo_request_op(mongo_request_t pkg) {
    return pkg->m_pro_head.m_op;
}

void mongo_request_set_op(mongo_request_t pkg, uint32_t op) {
    pkg->m_pro_head.m_op = op;
}

int mongo_request_set_size(mongo_request_t pkg, size_t size) {
    return dp_req_set_size(pkg->m_dp_req, sizeof(struct mongo_request) + size);
}

size_t mongo_request_size(mongo_request_t pkg) {
    return dp_req_size(pkg->m_dp_req) - sizeof(struct mongo_request);
}

size_t mongo_request_capacity(mongo_request_t pkg) {
    return dp_req_capacity(pkg->m_dp_req) - sizeof(struct mongo_request);
}

static void mongo_request_dump_i(write_stream_t stream, const char *data , int depth) {
    bson_iterator i;
    const char *key;
    int temp;
    bson_timestamp_t ts;
    char oidhex[25];
    bson scope;
    bson_iterator_from_buffer( &i, data);

    while(bson_iterator_next(&i)) {
        bson_type t = bson_iterator_type( &i);
        if ( t == 0 )
            break;
        key = bson_iterator_key( &i);

        for ( temp=0; temp<=depth; temp++ )
            stream_printf(stream,  "\t");
        stream_printf(stream,  "%s : %d \t " , key , t);
        switch ( t ) {
        case BSON_DOUBLE:
            stream_printf(stream,  "%f" , bson_iterator_double( &i ));
            break;
        case BSON_STRING:
            stream_printf(stream,  "%s" , bson_iterator_string( &i ));
            break;
        case BSON_SYMBOL:
            stream_printf(stream,  "SYMBOL: %s" , bson_iterator_string( &i ));
            break;
        case BSON_OID:
            bson_oid_to_string( bson_iterator_oid( &i ), oidhex);
            stream_printf(stream,  "%s" , oidhex);
            break;
        case BSON_BOOL:
            stream_printf(stream,  "%s" , bson_iterator_bool( &i ) ? "true" : "false");
            break;
        case BSON_DATE:
            stream_printf(stream,  "%ld" , ( long int )bson_iterator_date( &i ));
            break;
        case BSON_BINDATA:
            stream_printf(stream,  "BSON_BINDATA");
            break;
        case BSON_UNDEFINED:
            stream_printf(stream,  "BSON_UNDEFINED");
            break;
        case BSON_NULL:
            stream_printf(stream,  "BSON_NULL");
            break;
        case BSON_REGEX:
            stream_printf(stream,  "BSON_REGEX: %s", bson_iterator_regex( &i ));
            break;
        case BSON_CODE:
            stream_printf(stream,  "BSON_CODE: %s", bson_iterator_code( &i ));
            break;
        case BSON_CODEWSCOPE:
            stream_printf(stream,  "BSON_CODE_W_SCOPE: %s", bson_iterator_code( &i ));
            bson_init( &scope);
            bson_iterator_code_scope( &i, &scope);
            stream_printf(stream,  "\n\t SCOPE: ");
            bson_print( &scope);
            break;
        case BSON_INT:
            stream_printf(stream,  "%d" , bson_iterator_int( &i ));
            break;
        case BSON_LONG:
            stream_printf(stream,  "%lld" , ( uint64_t )bson_iterator_long( &i ));
            break;
        case BSON_TIMESTAMP:
            ts = bson_iterator_timestamp( &i);
            stream_printf(stream,  "i: %d, t: %d", ts.i, ts.t);
            break;
        case BSON_OBJECT:
        case BSON_ARRAY:
            stream_printf(stream,  "\n");
            mongo_request_dump_i(stream, bson_iterator_value(&i), depth + 1);
            break;
        default:
            stream_printf(stream, "can't print type : %d\n" , t);
        }
        stream_printf(stream, "\n");
    }
}

const char * mongo_request_dump(mongo_request_t req, mem_buffer_t buffer) {
    struct write_stream_buffer stream;

    mem_buffer_clear_data(buffer);

    write_stream_buffer_init(&stream, buffer);

    mongo_request_dump_i((write_stream_t)&stream, mongo_request_data(req), 0);

    stream_putc((write_stream_t)&stream, 0);

    return mem_buffer_make_continuous(buffer, 0);
}

CPE_HS_DEF_VAR(mongo_request_type_name, "mongo_request_type");

