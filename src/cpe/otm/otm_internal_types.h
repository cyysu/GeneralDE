#ifndef CPE_OTM_INTERNAL_TYPES_H
#define CPE_OTM_INTERNAL_TYPES_H
#include "cpe/utils/hash.h"
#include "cpe/utils/error.h"
#include "cpe/otm/otm_types.h"

struct otm_manage {
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;

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
