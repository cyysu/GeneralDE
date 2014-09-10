#include <assert.h>
#include <math.h>
#include "cpe/pal/pal_strings.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui_sprite_anim_camera_utils.h"

static void ui_sprite_anim_camera_adj_camera_in_limit_with_lock_pos_no_trace(
    ui_sprite_anim_camera_t camera, UI_SPRITE_2D_PAIR * camera_pos, float * camera_scale, UI_SPRITE_2D_PAIR const * lock_screen_pos)
{
    float left_world_x;
    float up_world_y;
    float right_world_x;
    float down_world_y;
    float lock_world_pos_x;
    float lock_world_pos_y;
    uint8_t updated = 0;

    assert(lock_screen_pos->x >= 0.0f && lock_screen_pos->x <= 1.0f);
    assert(lock_screen_pos->y >= 0.0f && lock_screen_pos->y <= 1.0f);

    if (!ui_sprite_anim_camera_have_limit(camera)) return;

    if (lock_screen_pos->x < camera->m_limit_lt.x) {
        assert(0);
        return;
    }

    lock_world_pos_x = camera_pos->x + camera->m_screen_size.x * lock_screen_pos->x * *camera_scale;
    lock_world_pos_y = camera_pos->y + camera->m_screen_size.y * lock_screen_pos->y * *camera_scale;

    left_world_x = camera->m_screen_size.x * lock_screen_pos->x * lock_screen_pos->x;
    if (lock_world_pos_x - left_world_x < camera->m_limit_lt.x) {
        *camera_scale = (lock_world_pos_x - camera->m_limit_lt.x) / (lock_screen_pos->x * camera->m_screen_size.x);
        updated = 1;
    }

    up_world_y = camera->m_screen_size.y * lock_screen_pos->y * lock_screen_pos->y;
    if (lock_world_pos_y - up_world_y < camera->m_limit_lt.y) {
        *camera_scale = (lock_world_pos_y - camera->m_limit_lt.y) / (lock_screen_pos->y * camera->m_screen_size.y);
        updated = 1;
    }

    right_world_x = camera->m_screen_size.x * (1 - lock_screen_pos->x) * (*camera_scale);
    if (lock_world_pos_x + right_world_x > camera->m_limit_rb.x) {
        *camera_scale = (camera->m_limit_rb.x - lock_world_pos_x) / ((1 - lock_screen_pos->x) * camera->m_screen_size.x);
        updated = 1;
    }

    down_world_y = camera->m_screen_size.y * (1 - lock_screen_pos->y) * (*camera_scale);
    if (lock_world_pos_y + down_world_y > camera->m_limit_rb.y) {
        *camera_scale = (camera->m_limit_rb.y - lock_world_pos_y) / ((1 - lock_screen_pos->y) * camera->m_screen_size.y);
        updated = 1;
    }

    if (updated) {
        camera_pos->x = lock_world_pos_x - camera->m_screen_size.x * lock_screen_pos->x * *camera_scale;
        camera_pos->y = lock_world_pos_y - camera->m_screen_size.y * lock_screen_pos->y * *camera_scale;
    }
}

static void ui_sprite_anim_camera_adj_camera_in_limit_with_lock_pos_trace_x(
    ui_sprite_anim_camera_t camera, UI_SPRITE_2D_PAIR * camera_pos, float * camera_scale,
    UI_SPRITE_2D_PAIR const * lock_screen_pos, UI_SPRITE_2D_PAIR const * lock_pos_in_world)
{
    if (!ui_sprite_anim_camera_have_limit(camera)) return;

    if (*camera_scale > camera->m_scale_max) *camera_scale = camera->m_scale_max;

    if (lock_pos_in_world->x > camera->m_limit_lt.x && lock_screen_pos->x > 0.0f) {
        float scale_left = (lock_pos_in_world->x - camera->m_limit_lt.x) / (camera->m_screen_size.x * lock_screen_pos->x);
        if (scale_left < camera->m_scale_min) scale_left = camera->m_scale_min;
        if (*camera_scale > scale_left) {
            *camera_scale = scale_left;
        }
    }

    if (lock_pos_in_world->x < camera->m_limit_rb.x && lock_screen_pos->x < 1.0f) {
        float scale_right = (camera->m_limit_rb.x - lock_pos_in_world->x) / (camera->m_screen_size.x * (1 - lock_screen_pos->x));
        if (scale_right < camera->m_scale_min) scale_right = camera->m_scale_min;
        if (*camera_scale > scale_right) {
            *camera_scale = scale_right;
        }
    }

    if (lock_pos_in_world->y > camera->m_limit_lt.y && lock_screen_pos->y > 0.0f) {
        float scale_up = (lock_pos_in_world->y - camera->m_limit_lt.y) / (camera->m_screen_size.y * lock_screen_pos->y);
        if (scale_up < camera->m_scale_min) scale_up = camera->m_scale_min;
        if (*camera_scale > scale_up) {
            *camera_scale = scale_up;
        }
    }

    if (lock_pos_in_world->y < camera->m_limit_rb.y && lock_screen_pos->y < 1.0f) {
        float scale_down = (camera->m_limit_rb.y - lock_pos_in_world->y) / (camera->m_screen_size.y * (1 - lock_screen_pos->y));
        if (scale_down < camera->m_scale_min) scale_down = camera->m_scale_min;
        if (*camera_scale > scale_down) {
            *camera_scale = scale_down;
            camera_pos->x = lock_pos_in_world->x - camera->m_screen_size.x * lock_screen_pos->x * *camera_scale;
        }
    }

    camera_pos->x = lock_pos_in_world->x - camera->m_screen_size.x * lock_screen_pos->x * *camera_scale;
    camera_pos->y = ui_sprite_anim_camera_trace_x2y(camera, camera_pos->x, *camera_scale);
}

void ui_sprite_anim_camera_adj_camera_in_limit_with_lock_pos(
    ui_sprite_anim_camera_t camera, UI_SPRITE_2D_PAIR * camera_pos, float * camera_scale,
    UI_SPRITE_2D_PAIR const * lock_pos_in_screen, UI_SPRITE_2D_PAIR const * lock_pos_in_world)
{
    switch(camera->m_trace_type) {
    case ui_sprite_anim_camera_trace_none:
        ui_sprite_anim_camera_adj_camera_in_limit_with_lock_pos_no_trace(camera, camera_pos, camera_scale, lock_pos_in_screen);
        break;
    case ui_sprite_anim_camera_trace_by_x:
        ui_sprite_anim_camera_adj_camera_in_limit_with_lock_pos_trace_x(camera, camera_pos, camera_scale, lock_pos_in_screen, lock_pos_in_world);
        break;
    case ui_sprite_anim_camera_trace_by_y:
        assert(0);
        break;
    default:
        break;
    }
}
