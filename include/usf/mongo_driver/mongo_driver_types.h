#ifndef USF_MONGO_DRIVER_SYSTEM_H
#define USF_MONGO_DRIVER_SYSTEM_H
#include "cpe/pal/pal_types.h"
#include "cpe/utils/hash_string.h"
#include "gd/app/app_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mongo_doc * mongo_doc_t;

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

typedef struct mongo_doc_it {
    mongo_doc_t (*next)(struct mongo_doc_it * it);
    char m_data[16];
} * mongo_doc_it_t;

typedef struct mongo_host_port * mongo_host_port_t;
typedef struct mongo_driver * mongo_driver_t;
typedef struct mongo_pkg * mongo_pkg_t;

#ifdef __cplusplus
}
#endif

#endif
