#ifndef UI_SPRITE_ANIM_BACKEND_H
#define UI_SPRITE_ANIM_BACKEND_H
#include "ui_sprite_anim_types.h"

#ifdef __cplusplus
extern "C" {
#endif


struct ui_sprite_anim_backend_def {
    void * m_ctx;
    /*global*/
    UI_SPRITE_2D_PAIR (*m_screen_size_fun)(void * ctx);

    /*group*/
    uint32_t (*m_create_group_fun)(void * ctx, const char * layer);
    void (*m_remove_group_fun)(void * ctx, uint32_t group_id);
    void (*m_pos_update_fun)(void * ctx, uint32_t group_id, UI_SPRITE_2D_PAIR  new_pos);
    void (*m_scale_update_fun)(void * ctx, uint32_t group_id, UI_SPRITE_2D_PAIR new_scale);
    void (*m_angle_update_fun)(void * ctx, uint32_t group_id, float new_angle);

    /*animation*/
    uint32_t (*m_start_fun)(void * ctx, uint32_t group_id, const char * res, uint8_t is_loop, int32_t start, int32_t end);
    void (*m_stop_fun)(void * ctx, uint32_t anim_id);
    uint8_t (*m_is_runing_fun)(void * ctx, uint32_t anim_id);
    int (*m_set_template_value)(void * ctx, uint32_t anim_id, const char * ctrl_name, const char * attr_name, const char * value);

    /*backend: camera*/
    void (*m_camera_update_fun)(void * ctx, UI_SPRITE_2D_PAIR pos, UI_SPRITE_2D_PAIR scale);
};

ui_sprite_anim_backend_t
ui_sprite_anim_backend_create(ui_sprite_anim_module_t module, ui_sprite_world_t world);
void ui_sprite_anim_backend_free(ui_sprite_world_t world);

ui_sprite_anim_backend_t ui_sprite_anim_backend_find(ui_sprite_world_t world);

void ui_sprite_anim_backend_set_op(ui_sprite_anim_backend_t backend, struct ui_sprite_anim_backend_def * def);
struct ui_sprite_anim_backend_def * ui_sprite_anim_backend_op(ui_sprite_anim_backend_t backend);

extern const char * UI_SPRITE_ANIM_BACKEND_NAME;

#ifdef __cplusplus
}
#endif

#endif
