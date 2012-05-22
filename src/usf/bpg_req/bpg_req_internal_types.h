#ifndef USF_BPG_REQ_INTERNAL_TYPES_H
#define USF_BPG_REQ_INTERNAL_TYPES_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/memory.h"
#include "usf/logic/logic_types.h"
#include "usf/bpg_pkg/bpg_pkg_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct bpg_req_manage {
    mem_allocrator_t m_alloc;
    gd_app_context_t m_app;
    logic_manage_t m_logic;
    int m_debug;
};

#ifdef __cplusplus
}
#endif

#endif
