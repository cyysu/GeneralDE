#ifndef UI_SPRITE_2D_MODULE_I_H
#define UI_SPRITE_2D_MODULE_I_H
#include "cpe/pal/pal_queue.h"
#include "gd/app/app_types.h"
#include "ui/sprite_2d/ui_sprite_2d_module.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_2d_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    ui_sprite_repository_t m_repo;
    ui_sprite_fsm_module_t m_fsm_module;
    error_monitor_t m_em;
    LPDRMETA m_meta_transform_data;
    LPDRMETA m_meta_search_result;
    int m_debug;
};

int ui_sprite_2d_add_angle_flip_x(ui_sprite_2d_module_t module);
void ui_sprite_2d_remove_angle_flip_x(ui_sprite_2d_module_t module);

#ifdef __cplusplus
}
#endif

#endif
