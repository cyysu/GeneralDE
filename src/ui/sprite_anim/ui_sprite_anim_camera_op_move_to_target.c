#include <assert.h>
#include "cpe/utils/math_ex.h"
#include "ui_sprite_anim_camera_op_i.h"
#include "ui_sprite_anim_camera_restrict_i.h"

void ui_sprite_anim_camera_op_update_move_to_target(ui_sprite_anim_camera_op_t camera_op, float delta) {
    ui_sprite_anim_camera_t camera = camera_op->m_camera;
    UI_SPRITE_ANIM_CAMERA_OP_MOVE_TO_TARGET * op = &camera_op->m_op.data.move_to_target;

    UI_SPRITE_2D_PAIR target_pos = {
        camera_op->m_op.data.move_to_target.target_pos.x
        , camera_op->m_op.data.move_to_target.target_pos.y
    };

    float target_scale = camera_op->m_op.data.move_to_target.target_scale;

    ui_sprite_anim_camera_restrict_adj(camera, &target_pos, &target_scale);

    /*没有速度和时间限制，则直接设置到位 */
    if (op->max_speed <= 0.0f && op->duration <= 0.0f) {
        ui_sprite_anim_camera_set_pos_and_scale_no_adj(camera, target_pos, target_scale);
    }
    else if (op->duration > 0.0f) {
        if (op->duration < delta) {
            ui_sprite_anim_camera_set_pos_and_scale_no_adj(camera, target_pos, target_scale);
            op->duration = 0.0f;
        }
        else {
            float percent = delta / op->duration;
            UI_SPRITE_2D_PAIR adj_pos;
            float adj_scale;

            adj_pos.x = camera->m_camera_pos.x + (target_pos.x - camera->m_camera_pos.x) * percent;
            adj_pos.y = camera->m_camera_pos.y + (target_pos.y - camera->m_camera_pos.y) * percent;
            adj_scale = camera->m_camera_scale + (target_scale - camera->m_camera_scale) * percent;

            ui_sprite_anim_camera_set_pos_and_scale_no_adj(camera, adj_pos, adj_scale);

            op->duration -= delta;
        }
    }
    else {
        float move_distance;
        float move_speed;

        assert(op->max_speed > 0.0f);

        move_distance = cpe_math_distance(target_pos.x, target_pos.y, camera->m_camera_pos.x, camera->m_camera_pos.y);
        move_speed = move_distance / delta;

        if (move_speed < op->max_speed) {
            ui_sprite_anim_camera_set_pos_and_scale_no_adj(camera, target_pos, target_scale);
        }
        else {
            float move_percent = op->max_speed / move_speed;
			float rad = atan2f(target_pos.y - camera->m_camera_pos.y, target_pos.x - camera->m_camera_pos.x);
            UI_SPRITE_2D_PAIR adj_pos;
            float adj_scale;
           
			move_distance = op->max_speed * delta;

			adj_pos.x = camera->m_camera_pos.x + cos(rad) * move_distance;
			adj_pos.y = camera->m_camera_pos.y + sin(rad) * move_distance;
            adj_scale = camera->m_camera_scale + (target_scale - camera->m_camera_scale) * move_percent;

            ui_sprite_anim_camera_set_pos_and_scale_no_adj(camera, adj_pos, adj_scale);
        }
    }

    if (fabs(camera->m_camera_pos.x - target_pos.x) < 0.01f
        && fabs(camera->m_camera_pos.y - target_pos.y) < 0.01f
        && fabs(camera->m_camera_scale - target_scale) < 0.001f)
    {
        camera_op->m_is_done = 1;
    }
}
