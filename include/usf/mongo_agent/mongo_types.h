#ifndef USF_MONGO_SYSTEM_H
#define USF_MONGO_SYSTEM_H
#include "cpe/pal/pal_types.h"
#include "cpe/utils/hash_string.h"
#include "gd/app/app_types.h"
#include "usf/logic/logic_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum mongo_agent_state {
    mongo_agent_state_disable = -1
    , mongo_agent_state_idle = 0
    , mongo_agent_state_connecting
    , mongo_agent_state_connected
    , mongo_agent_state_error
} mongo_agent_state_t;

typedef enum mongo_agent_error {
    mongo_agent_success = 0
    , mongo_agent_not_connected
} mongo_agent_error_t;

typedef struct mongo_host_port * mongo_host_port_t;
typedef struct mongo_agent * mongo_agent_t;
typedef struct mongo_request * mongo_request_t;

#ifdef __cplusplus
}
#endif

#endif
