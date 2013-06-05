#ifndef SVR_CENTER_SVR_OPS_H
#define SVR_CENTER_SVR_OPS_H
#include "cpe/utils/hash_string.h"
#include "center_svr_types.h"
#include "protocol/svr/center/svr_center_pro.h"

/*operations of center_svr */
center_svr_t
center_svr_create(
    gd_app_context_t app,
    const char * name,
    mem_allocrator_t alloc,
    error_monitor_t em);

int center_svr_start(center_svr_t svr, const char * ip, uint16_t port);
void center_svr_stop(center_svr_t svr);

int center_svr_load_svr_config(center_svr_t svr);

void center_svr_free(center_svr_t svr);

center_svr_t center_svr_find(gd_app_context_t app, cpe_hash_string_t name);
center_svr_t center_svr_find_nc(gd_app_context_t app, const char * name);

cpe_hash_string_t center_svr_name_hs(center_svr_t mgr);
const char * center_svr_name(center_svr_t svr);

int center_svr_set_ringbuf_size(center_svr_t svr, size_t capacity);
SVR_CENTER_PKG * center_svr_get_res_pkg_buff(center_svr_t svr, SVR_CENTER_PKG * req, size_t capacity);

int center_svr_init_clients_from_mem(center_svr_t svr, size_t capacity);
int center_svr_init_clients_from_shm(center_svr_t svr, int shm_key);

/*operations of center_cli_data */
center_cli_data_t center_cli_data_create(center_svr_t svr, center_cli_group_t group,  SVR_CENTER_CLI_RECORD * data);
center_cli_data_t center_cli_data_find(center_svr_t svr, uint16_t svr_type, uint16_t svr_id);
void center_cli_data_free(center_cli_data_t cli);
void center_cli_data_free_all(center_svr_t svr);

uint32_t center_cli_data_hash(center_cli_data_t cli);
int center_cli_data_eq(center_cli_data_t l, center_cli_data_t r);

/*operations of center_cli_group */
center_cli_group_t center_cli_group_create(center_svr_t svr, const char * svr_type_name, uint16_t svr_type);
center_cli_group_t center_cli_group_find(center_svr_t svr, uint16_t svr_type);
center_cli_group_t center_cli_group_lsearch_by_name(center_svr_t svr, const char * svr_type_name);
void center_cli_group_free(center_cli_group_t group);
void center_cli_group_free_all(center_svr_t svr);

uint32_t center_cli_group_hash(center_cli_group_t group);
int center_cli_group_eq(center_cli_group_t l, center_cli_group_t r);

/*operations of center_cli_relation */
center_cli_relation_t
center_cli_relation_create(center_cli_group_t provider, center_cli_group_t user);
void center_cli_relation_free(center_cli_relation_t relation);

/*operations of center_svr_conn */
center_svr_conn_t center_svr_conn_create(center_svr_t svr, int fd);
void center_svr_conn_free(center_svr_conn_t conn);
void center_svr_conn_free_all(center_svr_t svr);
int center_svr_conn_send(center_svr_conn_t conn, SVR_CENTER_PKG * pkg, size_t pkg_size);

void center_svr_conn_start_watch(center_svr_conn_t conn);
void center_svr_conn_link_node_r(center_svr_conn_t conn, ringbuffer_block_t blk);
void center_svr_conn_link_node_w(center_svr_conn_t conn, ringbuffer_block_t blk);
int center_svr_conn_alloc(ringbuffer_block_t * result, center_svr_t svr, center_svr_conn_t conn, size_t size);

uint32_t center_svr_conn_hash(center_svr_conn_t group);
int center_svr_conn_eq(center_svr_conn_t l, center_svr_conn_t r);

/*protocol process ops*/
typedef void (*center_svr_conn_op_t)(center_svr_conn_t conn, SVR_CENTER_PKG * pkg, size_t pkg_size);
void center_svr_conn_op_join(center_svr_conn_t conn, SVR_CENTER_PKG * pkg, size_t pkg_size);
void center_svr_conn_op_query_by_type(center_svr_conn_t conn, SVR_CENTER_PKG * pkg, size_t pkg_size);

#endif
