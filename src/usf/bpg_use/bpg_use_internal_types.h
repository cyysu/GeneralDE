#ifndef USF_BPG_USE_INTERNAL_TYPES_H
#define USF_BPG_USE_INTERNAL_TYPES_H
#include "usf/logic/logic_types.h"
#include "usf/bpg_use/bpg_use_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct bpg_use_sp {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    bpg_pkg_manage_t m_pkg_manage;
    error_monitor_t m_em;

    size_t m_pkg_buf_size;
    bpg_pkg_t m_pkg_buf;
    struct mem_buffer m_data_buf;

    bpg_pkg_dsp_t m_dsp;
};

struct bpg_use_proxy {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    logic_manage_t m_logic_mgr;

    int m_debug;
};

#ifdef __cplusplus
}
#endif

#endif
