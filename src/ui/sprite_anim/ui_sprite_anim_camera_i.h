#ifndef UI_SPRITE_ANIM_CAMERA_I_H
#define UI_SPRITE_ANIM_CAMERA_I_H
#include "ui/sprite_anim/ui_sprite_anim_camera.h"
#include "ui_sprite_anim_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_anim_camera {
    ui_sprite_anim_module_t m_module;
    UI_SPRITE_2D_PAIR m_screen_size;
    UI_SPRITE_2D_PAIR m_limit_lt;
    UI_SPRITE_2D_PAIR m_limit_rb;

    UI_SPRITE_2D_PAIR m_camera_base_pos;
    UI_SPRITE_2D_PAIR m_camera_base_scale;

    UI_SPRITE_2D_PAIR m_camera_pos;
    float m_camera_scale;
    UI_SPRITE_2D_PAIR m_camera_scale_pair;

    float m_scale_min;
    float m_scale_max;

    uint32_t m_max_op_id;
    uint32_t m_curent_op_id;

    /*镜头轨道 */
    enum ui_sprite_anim_camera_trace_type m_trace_type;
    UI_SPRITE_2D_PAIR m_trace_screen_pos;
    UI_SPRITE_2D_PAIR m_trace_world_pos;
    union {
        struct {
            float m_base_y;
            float m_dy_dx;
        } m_by_x;
        struct {
            float m_base_x;
            float m_dx_dy;
        } m_by_y;
    } m_trace_line;
};

int ui_sprite_anim_camera_pos_of_entity(UI_SPRITE_2D_PAIR * pos, ui_sprite_world_t world, uint32_t entity_id, const char * entity_name, uint8_t pos_of_entity);

uint32_t ui_sprite_anim_camera_start_op(ui_sprite_anim_camera_t camera);
void ui_sprite_anim_camera_stop_op(ui_sprite_anim_camera_t camera, uint32_t op_id);

void ui_sprite_anim_camera_adj_camera_in_limit(ui_sprite_anim_camera_t camera, UI_SPRITE_2D_PAIR * camera_pos, float * camera_scale);
void ui_sprite_anim_camera_adj_camera_in_limit_with_lock_pos(
    ui_sprite_anim_camera_t camera, UI_SPRITE_2D_PAIR * camera_pos, float * camera_scale,
    UI_SPRITE_2D_PAIR const * lock_pos_in_screen, UI_SPRITE_2D_PAIR const * lock_pos_in_world);


/*根据锁定规则，camera位置中的一个计算另外一个 */
float ui_sprite_anim_camera_trace_x2y(ui_sprite_anim_camera_t camera, float camera_x, float scale);
float ui_sprite_anim_camera_trace_y2x(ui_sprite_anim_camera_t camera, float camera_y, float scale);

/*根据屏幕锁定点的世界坐标和屏幕坐标中的一个，计算另外一个屏幕坐标 */
float ui_sprite_anim_camera_screen_x2y_lock_x(ui_sprite_anim_camera_t camera, float screen_x, UI_SPRITE_2D_PAIR workd_pos, float scale);
float ui_sprite_anim_camera_screen_y2x_lock_y(ui_sprite_anim_camera_t camera, float screen_y, UI_SPRITE_2D_PAIR workd_pos, float scale);
        
#ifdef __cplusplus
}
#endif

#endif
