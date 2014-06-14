#ifndef UI_SPRITE_ANIM_MODULE_I_H
#define UI_SPRITE_ANIM_MODULE_I_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/buffer.h"
#include "gd/app/app_types.h"
#include "ui/sprite_anim/ui_sprite_anim_module.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_sprite_anim_camera_op * ui_sprite_anim_camera_op_t;
typedef TAILQ_HEAD(ui_sprite_anim_camera_op_list, ui_sprite_anim_camera_op) ui_sprite_anim_camera_op_list_t;
typedef TAILQ_HEAD(ui_sprite_anim_camera_restrict_list, ui_sprite_anim_camera_restrict) ui_sprite_anim_camera_restrict_list_t;

typedef TAILQ_HEAD(ui_sprite_anim_def_list, ui_sprite_anim_def) ui_sprite_anim_def_list_t;

typedef TAILQ_HEAD(ui_sprite_anim_group_list, ui_sprite_anim_group) ui_sprite_anim_group_list_t;
typedef TAILQ_HEAD(ui_sprite_anim_template_list, ui_sprite_anim_template) ui_sprite_anim_template_list_t;
typedef TAILQ_HEAD(ui_sprite_anim_template_binding_list, ui_sprite_anim_template_binding) ui_sprite_anim_template_binding_list_t;

typedef struct ui_sprite_anim_runing * ui_sprite_anim_runing_t;
typedef TAILQ_HEAD(ui_sprite_anim_runing_list, ui_sprite_anim_runing) ui_sprite_anim_runing_list_t;

struct ui_sprite_anim_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    ui_sprite_repository_t m_repo;
    ui_sprite_fsm_module_t m_fsm_module;
    error_monitor_t m_em;
    int m_debug;

    struct mem_buffer m_dump_buffer;
};

#ifdef __cplusplus
}
#endif

#endif
