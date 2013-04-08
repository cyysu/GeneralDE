#ifndef SVR_CENTER_AGENT_INTERNAL_TYPES_H
#define SVR_CENTER_AGENT_INTERNAL_TYPES_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/net/net_types.h"
#include "usf/bpg_pkg/bpg_pkg_types.h"
#include "svr/center/center_agent_types.h"

struct center_agent {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    int m_debug;
    bpg_pkg_manage_t m_pkg_manager;

    uint32_t m_read_chanel_size;
    uint32_t m_write_chanel_size;
    net_connector_t m_connector;
};

#endif
