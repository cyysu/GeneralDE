#ifndef SVR_CENTER_AGENT_INTERNAL_TYPES_H
#define SVR_CENTER_AGENT_INTERNAL_TYPES_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/net/net_types.h"
#include "gd/dr_cvt/dr_cvt_types.h"
#include "svr/center/center_agent_types.h"

struct center_agent {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    int m_debug;

    dr_cvt_t m_cvt;

    uint32_t m_read_chanel_size;
    uint32_t m_write_chanel_size;
    net_connector_t m_connector;
};

#endif
