#ifndef UI_SPRITE_SPINE_MODULE_I_H
#define UI_SPRITE_SPINE_MODULE_I_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/buffer.h"
#include "gd/app/app_types.h"
#include "ui/sprite_fsm/ui_sprite_fsm_types.h"
#include "ui/sprite_spine/ui_sprite_spine_module.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_spine_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    ui_sprite_repository_t m_repo;
    ui_sprite_fsm_module_t m_fsm_module;
    error_monitor_t m_em;
    int m_debug;
};

#ifdef __cplusplus
}
#endif

#endif
