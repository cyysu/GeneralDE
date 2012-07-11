#ifndef GD_OM_TOOLS_GENERATE_OPS_H
#define GD_OM_TOOLS_GENERATE_OPS_H
#include "cpe/utils/stream.h"
#include "cpe/utils/error.h"
#include "cpe/dr/dr_types.h"
#include "gd/om/om_types.h"
#include "gd/om_grp/om_grp_types.h"
#include "gd/om/om_manage.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct gd_om_tool_env {
    gd_om_mgr_t m_om_mgr;
    om_grp_meta_t m_om_grp_meta;
    LPDRMETALIB m_input_metalib;
    error_monitor_t m_em;
    int m_shm_id;
} * cpe_dr_tool_env_t;

int gd_om_tool_shm_init(struct gd_om_tool_env * env);

#ifdef __cplusplus
}
#endif

#endif
