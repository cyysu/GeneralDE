#ifndef GD_OM_TOOLS_GENERATE_OPS_H
#define GD_OM_TOOLS_GENERATE_OPS_H
#include "cpe/utils/stream.h"
#include "cpe/utils/error.h"
#include "cpe/dr/dr_types.h"
#include "cpe/pom/pom_types.h"
#include "cpe/pom/pom_manage.h"
#include "cpe/pom_grp/pom_grp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct pom_tool_env {
    pom_mgr_t m_pom_mgr;
    pom_grp_meta_t m_pom_grp_meta;
    LPDRMETALIB m_input_metalib;
    error_monitor_t m_em;
    int m_shm_id;
} * cpe_dr_tool_env_t;

int pom_tool_shm_init(struct pom_tool_env * env);
int pom_tool_generate_lib_c(struct pom_tool_env * env, const char * filename);

#ifdef __cplusplus
}
#endif

#endif
