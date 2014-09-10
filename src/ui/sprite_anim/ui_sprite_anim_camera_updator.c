#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "ui/sprite_2d/ui_sprite_2d_utils.h"
#include "ui_sprite_anim_camera_updator.h"

void ui_sprite_anim_camera_updator_set_max_speed(ui_sprite_anim_camera_updator_t updator, float max_speed) {
    updator->m_max_speed = max_speed;
}

void ui_sprite_anim_camera_updator_stop(ui_sprite_anim_camera_updator_t updator, ui_sprite_anim_camera_t camera) {
    if (updator->m_curent_op_id) {
        if (camera) ui_sprite_anim_camera_stop_op(camera, updator->m_curent_op_id);
        updator->m_curent_op_id = 0;
    }

    updator->m_duration = 0.0f;
}

static void ui_sprite_anim_camera_updator_update_data(ui_sprite_anim_camera_updator_t updator, ui_sprite_anim_camera_t camera) {
    float diff_lt;
    float diff_rb;

    updator->m_origin_rect.lt = camera->m_camera_pos;
    updator->m_origin_rect.rb.x = updator->m_origin_rect.lt.x + camera->m_screen_size.x * camera->m_camera_scale;
    updator->m_origin_rect.rb.y = updator->m_origin_rect.lt.y + camera->m_screen_size.y * camera->m_camera_scale;

    diff_lt = cpe_math_distance(
        updator->m_origin_rect.lt.x, updator->m_origin_rect.lt.y,
        updator->m_target_rect.lt.x, updator->m_target_rect.lt.y);

    diff_rb = cpe_math_distance(
        updator->m_origin_rect.rb.x, updator->m_origin_rect.rb.y,
        updator->m_target_rect.rb.x, updator->m_target_rect.rb.y);

    updator->m_duration =
        diff_lt > diff_rb
        ? diff_lt / updator->m_max_speed
        : diff_rb / updator->m_max_speed;

    updator->m_runing_time = 0.0f;
}

void ui_sprite_anim_camera_updator_set_camera(
    ui_sprite_anim_camera_updator_t updator, ui_sprite_anim_camera_t camera, UI_SPRITE_2D_PAIR pos, float scale)
{
    UI_SPRITE_2D_RECT target_rect;

    if (updator->m_max_speed == 0.0f) {
        assert(updator->m_curent_op_id == 0);

        updator->m_curent_op_id = ui_sprite_anim_camera_start_op(camera);
        ui_sprite_anim_camera_set_pos_and_scale(camera, pos, scale);
        ui_sprite_anim_camera_stop_op(camera, updator->m_curent_op_id);
        updator->m_curent_op_id = 0;

        return;
    }

    target_rect.lt = pos;
    target_rect.rb.x = pos.x + camera->m_screen_size.x * scale;
    target_rect.rb.y = pos.y + camera->m_screen_size.y * scale;

    if (updator->m_duration > 0.0f && ui_sprite_2d_rect_eq(&target_rect, &updator->m_target_rect, 0.1)) {
        return;
    }

    updator->m_target_rect = target_rect;
    ui_sprite_anim_camera_updator_update_data(updator, camera);

    if (updator->m_duration == 0.0f) {
        ui_sprite_anim_camera_updator_stop(updator, camera);
        return;
    }

    if (updator->m_curent_op_id == 0) {
        updator->m_curent_op_id = ui_sprite_anim_camera_start_op(camera);
    }

    assert(updator->m_curent_op_id == camera->m_curent_op_id);
}

void ui_sprite_anim_camera_updator_update(ui_sprite_anim_camera_updator_t updator, ui_sprite_anim_camera_t camera, float delta) {
    float percent;
    UI_SPRITE_2D_PAIR pos_lt;
    UI_SPRITE_2D_PAIR pos_rb;
    float scale;

    if (!ui_sprite_anim_camera_updator_is_runing(updator)) return;

    assert(updator->m_duration > 0.0f);

    if (updator->m_curent_op_id != camera->m_curent_op_id) {
        if (camera->m_curent_op_id != 0) return;
        updator->m_curent_op_id = ui_sprite_anim_camera_start_op(camera);
        ui_sprite_anim_camera_updator_update_data(updator, camera);

        if (updator->m_duration == 0.0f) {
            ui_sprite_anim_camera_updator_stop(updator, camera);
            return;
        }
    }

    updator->m_runing_time += delta;

    percent = 
        updator->m_runing_time > updator->m_duration
        ? 1.0f
        : (updator->m_runing_time / updator->m_duration);

    percent = ui_percent_decorator_decorate(&updator->m_decorator, percent);

    pos_lt.x = updator->m_origin_rect.lt.x + (updator->m_target_rect.lt.x - updator->m_origin_rect.lt.x) * percent;
    pos_lt.y = updator->m_origin_rect.lt.y + (updator->m_target_rect.lt.y - updator->m_origin_rect.lt.y) * percent;

    pos_rb.x = updator->m_origin_rect.rb.x + (updator->m_target_rect.rb.x - updator->m_origin_rect.rb.x) * percent;
    pos_rb.y = updator->m_origin_rect.rb.y + (updator->m_target_rect.rb.y - updator->m_origin_rect.rb.y) * percent;

    scale = fabs(pos_rb.x - pos_lt.x) / camera->m_screen_size.x;

    /* /\* printf("xxxxx: duration = %f, percent=%f, pos-in-world=(%f,%f), pos-in-screen=(%f,%f), " *\/ */
    /* /\*        "origin-pos=(%f,%f), camera-pos=(%f,%f), camera-scale=%f\n", *\/ */
    /* /\*        move->m_duration, percent, move->m_pos_in_world.x, move->m_pos_in_world.y, *\/ */
    /* /\*        move->m_pos_in_screen.x, move->m_pos_in_screen.y, *\/ */
    /* /\*        move->m_camera_orig_pos.x, move->m_camera_orig_pos.y, *\/ */
    /* /\*        target_camera_pos.x, target_camera_pos.y, target_camera_scale); *\/ */

    ui_sprite_anim_camera_set_pos_and_scale(camera, pos_lt, scale);

    if (percent >= 1.0f) ui_sprite_anim_camera_updator_stop(updator, camera);
}

uint8_t ui_sprite_anim_camera_updator_is_runing(ui_sprite_anim_camera_updator_t updator) {
    return updator->m_curent_op_id != 0;
}
