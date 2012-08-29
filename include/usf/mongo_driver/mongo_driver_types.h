#ifndef USF_MONGO_DRIVER_SYSTEM_H
#define USF_MONGO_DRIVER_SYSTEM_H
#include "cpe/pal/pal_types.h"
#include "cpe/utils/hash_string.h"
#include "gd/app/app_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum mongo_driver_state {
    mongo_driver_state_disable = -1
    , mongo_driver_state_connecting
    , mongo_driver_state_connected
    , mongo_driver_state_error
} mongo_driver_state_t;

typedef enum mongo_driver_error {
    mongo_driver_success = 0
    , mongo_driver_not_connected
} mongo_driver_error_t;

typedef enum mongo_db_op {
    mongo_db_op_replay = 1,
    mongo_db_op_msg = 1000,
    mongo_db_op_update = 2001,
    mongo_db_op_insert = 2002,
    mongo_db_op_query = 2004,
    mongo_db_op_get_more = 2005,
    mongo_db_op_delete = 2006,
    mongo_db_op_kill_cursors = 2007
} mongo_db_op_t;

typedef struct mongo_host_port * mongo_host_port_t;
typedef struct mongo_driver * mongo_driver_t;
typedef struct mongo_pkg * mongo_pkg_t;

#ifdef __cplusplus
}
#endif

#endif
