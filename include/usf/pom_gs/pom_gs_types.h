#ifndef USF_POM_GS_TYPES_H
#define USF_POM_GS_TYPES_H
#include "cpe/utils/error.h"
#include "cpe/utils/hash_string.h"
#include "cpe/cfg/cfg_types.h"
#include "cpe/pom_grp/pom_grp_types.h"
#include "usf/logic/logic_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct pom_gs_agent * pom_gs_agent_t;

typedef struct pom_gs_agent_backend {
    const char * name;
    int (*insert)(pom_grp_obj_mgr_t obj_mgr, pom_grp_obj_t obj, logic_require_t require, void * ctx);
} * pom_gs_agent_backend_t;

#ifdef __cplusplus
}
#endif

#endif
