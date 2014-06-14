#ifndef UI_SPRITE_ANIM_CAMERA_RESTRICTION_H
#define UI_SPRITE_ANIM_CAMERA_RESTRICTION_H
#include "ui_sprite_anim_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*ui_sprite_anim_camera_restrict_fun_t)(void * ctx, ui_sprite_anim_camera_t camera, UI_SPRITE_2D_PAIR * pos, float * scale);

ui_sprite_anim_camera_restrict_t
ui_sprite_anim_camera_restrict_create(
    ui_sprite_anim_camera_t camera,
    const char * name,
    ui_sprite_anim_camera_restrict_fun_t fun,
    void * ctx);

ui_sprite_anim_camera_restrict_t
ui_sprite_anim_camera_restrict_find(
    ui_sprite_anim_camera_t camera, const char * name);

void ui_sprite_anim_camera_restrict_free(ui_sprite_anim_camera_restrict_t camera_restrict);

void ui_sprite_anim_camera_restrict_adj(
    ui_sprite_anim_camera_t camera, UI_SPRITE_2D_PAIR * pos, float * scale);

#ifdef __cplusplus
}
#endif

#endif
