#ifndef SVR_SET_SVR_OPS_H
#define SVR_SET_SVR_OPS_H
#include "cpe/utils/hash_string.h"
#include "set_svr_types.h"
#include "protocol/svr/set/svr_set_pro.h"

/*operations of set_svr */
set_svr_t
set_svr_create(
    gd_app_context_t app, const char * name, 
    const char * repository_root, const char * set_type, uint16_t set_id,
    mem_allocrator_t alloc, error_monitor_t em);
void set_svr_free(set_svr_t svr);

set_svr_t set_svr_find(gd_app_context_t app, cpe_hash_string_t name);
set_svr_t set_svr_find_nc(gd_app_context_t app, const char * name);

cpe_hash_string_t set_svr_name_hs(set_svr_t mgr);
const char * set_svr_name(set_svr_t svr);

int set_svr_set_ringbuf_size(set_svr_t svr, size_t capacity);

ptr_int_t set_svr_dispatch_tick(void * ctx, ptr_int_t arg);

ringbuffer_block_t set_svr_ringbuffer_alloc(set_svr_t svr, int size, uint32_t id);

uint32_t set_svr_cur_time(set_svr_t svr);

/*operations of set_svr_svr_type*/
set_svr_svr_type_t set_svr_svr_type_create(set_svr_t svr, const char * svr_type);
void set_svr_svr_type_free(set_svr_svr_type_t svr_type);
void set_svr_svr_type_free_all(set_svr_t svr);

set_svr_svr_type_t set_svr_svr_type_find_by_id(set_svr_t svr, uint16_t svr_type_id);
set_svr_svr_type_t set_svr_svr_type_find_by_name(set_svr_t svr, const char * svr_type_name);

uint32_t set_svr_svr_type_hash_by_id(set_svr_svr_type_t o);
int set_svr_svr_type_eq_by_id(set_svr_svr_type_t l, set_svr_svr_type_t r);

uint32_t set_svr_svr_type_hash_by_name(set_svr_svr_type_t o);
int set_svr_svr_type_eq_by_name(set_svr_svr_type_t l, set_svr_svr_type_t r);

int set_svr_set_pkg_meta(set_svr_t svr, dp_req_t body, set_svr_svr_type_t to_svr_type, set_svr_svr_type_t from_svr_type);

/*operations of set_svr_svr*/
set_svr_svr_t set_svr_svr_create(set_svr_t svr, set_svr_svr_type_t type, uint16_t svr_id, enum set_svr_svr_category category);
void set_svr_svr_free(set_svr_svr_t svr_svr);
void set_svr_svr_free_all(set_svr_t svr);

void set_svr_svr_set_category(set_svr_svr_t svr_svr, enum set_svr_svr_category category);

set_svr_svr_t set_svr_svr_find(set_svr_t svr, uint16_t svr_type_id, uint16_t svr_id);

uint32_t set_svr_svr_hash(set_svr_svr_t o);
int set_svr_svr_eq(set_svr_svr_t l, set_svr_svr_t r);

#endif
