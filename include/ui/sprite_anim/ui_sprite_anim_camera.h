#ifndef UI_SPRITE_ANIM_CAMERA_H
#define UI_SPRITE_ANIM_CAMERA_H
#include "ui_sprite_anim_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_ANIM_CAMERA_TYPE_NAME;

ui_sprite_anim_camera_t ui_sprite_anim_camera_create(
    ui_sprite_anim_module_t module, ui_sprite_world_t world);
void ui_sprite_anim_camera_free(ui_sprite_anim_camera_t camera);
ui_sprite_anim_camera_t ui_sprite_anim_camera_find(ui_sprite_world_t world);

UI_SPRITE_2D_PAIR ui_sprite_anim_camera_screen_size(ui_sprite_anim_camera_t camera);
void ui_sprite_anim_camera_set_screen_size(ui_sprite_anim_camera_t camera, UI_SPRITE_2D_PAIR screen_size);

/*镜头位置控制 */
float ui_sprite_anim_camera_scale(ui_sprite_anim_camera_t camera);
UI_SPRITE_2D_PAIR ui_sprite_anim_camera_scale_pair(ui_sprite_anim_camera_t camera);
UI_SPRITE_2D_PAIR ui_sprite_anim_camera_pos(ui_sprite_anim_camera_t camera);
void ui_sprite_anim_camera_set_pos_and_scale(ui_sprite_anim_camera_t camera, UI_SPRITE_2D_PAIR pos, float scale);
void ui_sprite_anim_camera_rect(ui_sprite_anim_camera_t camera, UI_SPRITE_2D_RECT * rect);

/*镜头范围限制的设定 */
UI_SPRITE_2D_PAIR ui_sprite_anim_camera_limit_lt(ui_sprite_anim_camera_t camera);
UI_SPRITE_2D_PAIR ui_sprite_anim_camera_limit_rb(ui_sprite_anim_camera_t camera);
int ui_sprite_anim_camera_set_limit(ui_sprite_anim_camera_t camera, UI_SPRITE_2D_PAIR lt, UI_SPRITE_2D_PAIR br);
int8_t ui_sprite_anim_camera_have_limit(ui_sprite_anim_camera_t camera);

/* 镜头缩放限定 */
int ui_sprite_anim_camera_set_scale_range(ui_sprite_anim_camera_t camera, float scale_min, float scale_max);
float ui_sprite_anim_camera_scale_min(ui_sprite_anim_camera_t camera);
float ui_sprite_anim_camera_scale_max(ui_sprite_anim_camera_t camera);

/*镜头相关的处理函数 */
UI_SPRITE_2D_PAIR ui_sprite_anim_camera_center_pos(ui_sprite_anim_camera_t camera);

UI_SPRITE_2D_PAIR ui_sprite_anim_camera_screen_to_world(ui_sprite_anim_camera_t camera, UI_SPRITE_2D_PAIR pos);
UI_SPRITE_2D_PAIR ui_sprite_anim_camera_world_to_screen(ui_sprite_anim_camera_t camera, UI_SPRITE_2D_PAIR pos);

UI_SPRITE_2D_PAIR ui_sprite_anim_camera_calc_pos_from_pos_in_screen(
    ui_sprite_anim_camera_t camera, UI_SPRITE_2D_PAIR pos_in_world, UI_SPRITE_2D_PAIR pos_on_screen, float scale);

/*镜头轨道 */
enum ui_sprite_anim_camera_trace_type {
    ui_sprite_anim_camera_trace_none,
    ui_sprite_anim_camera_trace_by_x,
    ui_sprite_anim_camera_trace_by_y
};

int ui_sprite_anim_camera_set_trace(
    ui_sprite_anim_camera_t camera, enum ui_sprite_anim_camera_trace_type type,
    UI_SPRITE_2D_PAIR screen_pos, UI_SPRITE_2D_PAIR world_pos_a, UI_SPRITE_2D_PAIR world_pos_b);

void ui_sprite_anim_camera_remove_trace(ui_sprite_anim_camera_t camera);

#ifdef __cplusplus
}
#endif

#endif
