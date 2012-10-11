#ifndef USF_LOGIC_LS_INTERNAL_TYPES_H
#define USF_LOGIC_LS_INTERNAL_TYPES_H
#include "usf/logic_ls/logic_ls_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct logic_local_svr {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;

    int m_debug;
};

#ifdef __cplusplus
}
#endif

#endif
