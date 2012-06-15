#ifndef GD_OTM_INTERNAL_TYPES_H
#define GD_OTM_INTERNAL_TYPES_H
#include "cpe/utils/hash.h"
#include "gd/app/app_types.h"
#include "gd/otm/otm_types.h"

struct otm_manage {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    int64_t m_tick_span;
    int m_debug;

    struct cpe_hash_table m_timers;
};

struct otm_timer {
    otm_manage_t m_mgr;
    otm_timer_id_t m_id;
    const char * m_name;
    otm_process_fun_t m_process;
    void * m_process_ctx;

    struct cpe_hash_entry m_hh;
};

#endif
