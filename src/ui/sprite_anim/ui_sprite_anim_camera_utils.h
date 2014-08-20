#ifndef UI_SPRITE_ANIM_CAMERA_UTILS_H
#define UI_SPRITE_ANIM_CAMERA_UTILS_H
#include "ui_sprite_anim_camera_i.h"

#ifdef __cplusplus
extern "C" {
#endif

/*检查当前镜头坐标是否在屏幕范围以内 */
enum ui_sprite_anim_camera_limit_check_result {
    ui_sprite_anim_camera_limit_check_result_small = -1,
    ui_sprite_anim_camera_limit_check_result_ok = 0,
    ui_sprite_anim_camera_limit_check_result_bigger = 1,
};
enum ui_sprite_anim_camera_limit_check_result
ui_sprite_anim_camera_check_in_area(
    ui_sprite_anim_camera_t camera, UI_SPRITE_2D_PAIR const * camera_pos, float camera_scale);

/*数据一个点的世界坐标、屏幕坐标，以及scale，计算镜头位置 */
UI_SPRITE_2D_PAIR ui_sprite_anim_camera_calc_pos(
   ui_sprite_anim_camera_t camera, UI_SPRITE_2D_PAIR world_pos, UI_SPRITE_2D_PAIR screen_pos, float scale);

/*根据一个屏幕的点，计算确保屏幕在可视范围内的最大scale */
float ui_sprite_anim_camera_max_scale_from_point(
    ui_sprite_anim_camera_t camera,
    UI_SPRITE_2D_PAIR world_pos, UI_SPRITE_2D_PAIR screen_pos);

/*根据屏幕百分比坐标，计算屏幕的世界坐标 */
void ui_sprite_anim_camera_screen_world_rect(
    UI_SPRITE_2D_RECT * screen_world, UI_SPRITE_2D_RECT const * screen_percent, 
    ui_sprite_anim_camera_t camera, UI_SPRITE_2D_PAIR const * camera_pos, float camera_scale);

extern UI_SPRITE_2D_RECT g_full_screen_percent;
        
#ifdef __cplusplus
}
#endif

#endif
