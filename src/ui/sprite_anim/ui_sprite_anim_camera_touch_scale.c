#include <assert.h>
#include "cpe/utils/math_ex.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui_sprite_anim_camera_touch_i.h"
#include "ui_sprite_anim_camera_utils.h"

static void ui_sprite_anim_camera_touch_scale_adj_by_trace_x(
    ui_sprite_anim_camera_touch_t touch, ui_sprite_entity_t entity, 
    ui_sprite_anim_camera_t camera, UI_SPRITE_2D_PAIR * camera_pos, float * camera_scale,
    UI_SPRITE_2D_PAIR * lock_screen_pos, UI_SPRITE_2D_PAIR * lock_world_pos)
{
    ui_sprite_anim_module_t module = touch->m_module;
    UI_SPRITE_2D_RECT screen_rect;

    camera_pos->y = ui_sprite_anim_camera_trace_x2y(camera, camera_pos->x, *camera_scale);
    lock_screen_pos->y = camera->m_trace_line.m_by_x.m_base_y + camera->m_trace_line.m_by_x.m_dy_dx * lock_screen_pos->x;
    lock_world_pos->y = camera->m_trace_world_pos.y + (lock_world_pos->x - camera->m_trace_world_pos.x) * camera->m_trace_line.m_by_x.m_dy_dx;

    if (*camera_scale > camera->m_scale_max) *camera_scale = camera->m_scale_max;
    if (*camera_scale < camera->m_scale_min) *camera_scale = camera->m_scale_min;
    
    ui_sprite_anim_camera_screen_world_rect(
        &screen_rect, &g_full_screen_percent, camera, camera_pos, *camera_scale);

    if (lock_world_pos->y > camera->m_limit_lt.y && lock_screen_pos->y > 0.0f) {
        float scale_up = (lock_world_pos->y - camera->m_limit_lt.y) / (camera->m_screen_size.y * lock_screen_pos->y);
        if (scale_up < camera->m_scale_min) scale_up = camera->m_scale_min;
        if (*camera_scale > scale_up) {
            if (ui_sprite_entity_debug(entity)) {
                CPE_INFO(
                    module->m_em, "entity %d(%s): camera-touch: on-scale: 2-finter: trace-by-x: limit by up: %f ==> %f"
                    "track=(%f-%f), init-screen=(%f,%f)-(%f,%f), limit=(%f,%f)-(%f,%f), lock-world-pos=(%f,%f), lock-screen-pos=(%f,%f)",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                    *camera_scale, scale_up,
                    camera->m_trace_line.m_by_x.m_base_y, camera->m_trace_line.m_by_x.m_dy_dx,
                    touch->m_init_camera_rect.lt.x, touch->m_init_camera_rect.lt.y, touch->m_init_camera_rect.rb.x, touch->m_init_camera_rect.rb.y,
                    camera->m_limit_lt.x, camera->m_limit_lt.y, camera->m_limit_rb.x, camera->m_limit_rb.y,
                    lock_world_pos->x, lock_world_pos->y, lock_screen_pos->x, lock_screen_pos->y);
            }

            *camera_scale = scale_up;
            camera_pos->x = lock_world_pos->x - camera->m_screen_size.x * lock_screen_pos->x * *camera_scale;
        }
    }

    if (lock_world_pos->y < camera->m_limit_rb.y && lock_screen_pos->y < 1.0f) {
        float scale_down = (camera->m_limit_rb.y - lock_world_pos->y) / (camera->m_screen_size.y * (1 - lock_screen_pos->y));
        if (scale_down < camera->m_scale_min) scale_down = camera->m_scale_min;
        if (*camera_scale > scale_down) {
            if (ui_sprite_entity_debug(entity)) {
                CPE_INFO(
                    module->m_em, "entity %d(%s): camera-touch: on-scale: 2-finter: trace-by-x: limit by down: %f ==> %f"
                    "track=(%f-%f), init-screen=(%f,%f)-(%f,%f), limit=(%f,%f)-(%f,%f), lock-world-pos=(%f,%f), lock-screen-pos=(%f,%f)",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                    *camera_scale, scale_down,
                    camera->m_trace_line.m_by_x.m_base_y, camera->m_trace_line.m_by_x.m_dy_dx,
                    touch->m_init_camera_rect.lt.x, touch->m_init_camera_rect.lt.y, touch->m_init_camera_rect.rb.x, touch->m_init_camera_rect.rb.y,
                    camera->m_limit_lt.x, camera->m_limit_lt.y, camera->m_limit_rb.x, camera->m_limit_rb.y,
                    lock_world_pos->x, lock_world_pos->y, lock_screen_pos->x, lock_screen_pos->y);
            }

            *camera_scale = scale_down;
            camera_pos->x = lock_world_pos->x - camera->m_screen_size.x * lock_screen_pos->x * *camera_scale;
        }
    }
    camera_pos->y = ui_sprite_anim_camera_trace_x2y(camera, camera_pos->x, *camera_scale);

    if (camera_pos->x < camera->m_limit_lt.x) {
        camera_pos->x = camera->m_limit_lt.x;
    }
    else if ((camera_pos->x + camera->m_screen_size.x * *camera_scale) > camera->m_limit_rb.x) {
        camera_pos->x = camera->m_limit_rb.x - camera->m_screen_size.x * *camera_scale;
    }
}

int ui_sprite_anim_camera_touch_on_scale_2_finter_trace_by_x(
    ui_sprite_anim_camera_touch_t touch, ui_sprite_anim_camera_t camera, ui_sprite_entity_t entity,
    UI_SPRITE_2D_PAIR * target_pos, float * target_scale,
    UI_SPRITE_EVT_ANIM_CAMERA_TOUCH_SCALE const * evt_data)
{
    ui_sprite_anim_module_t module = touch->m_module;
    uint8_t l_pos = 0;
    uint8_t r_pos = 1;
    float world_x_l;
    float world_x_r;
    UI_SPRITE_2D_PAIR lock_pos_in_screen;
    UI_SPRITE_2D_PAIR lock_pos_in_world;

    if (fabs(evt_data->start_screen_pos[0].x - evt_data->start_screen_pos[1].x) < 5) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera-touch: on-scale: 2-finter: trace-by-x: screen pos too close in x, %f and %f!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
            evt_data->start_screen_pos[0].x, evt_data->start_screen_pos[1].x);
        return -1;
    }

    if (evt_data->start_screen_pos[0].x < evt_data->start_screen_pos[1].x) {
        l_pos = 0; r_pos = 1;
    }
    else {
        l_pos = 1; r_pos = 0;
    }

    if (evt_data->curent_screen_pos[l_pos].x >= evt_data->curent_screen_pos[r_pos].x) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera-touch: on-scale: 2-finter: trace-by-x: finger swiped!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    world_x_l = touch->m_init_camera_rect.lt.x
        + (touch->m_init_camera_rect.rb.x - touch->m_init_camera_rect.lt.x) 
        * (evt_data->start_screen_pos[l_pos].x / camera->m_screen_size.x);
        
    world_x_r = touch->m_init_camera_rect.lt.x
        + (touch->m_init_camera_rect.rb.x - touch->m_init_camera_rect.lt.x)
        * (evt_data->start_screen_pos[r_pos].x / camera->m_screen_size.x);

    assert(world_x_r > world_x_l);

    *target_scale = (world_x_r - world_x_l) / (evt_data->curent_screen_pos[r_pos].x - evt_data->curent_screen_pos[l_pos].x);

    target_pos->x = world_x_l - (evt_data->curent_screen_pos[l_pos].x * *target_scale);

    lock_pos_in_screen.x = evt_data->curent_screen_pos[l_pos].x / camera->m_screen_size.x;
    lock_pos_in_screen.y = evt_data->curent_screen_pos[l_pos].y / camera->m_screen_size.y;

    lock_pos_in_world.x = world_x_l;
    lock_pos_in_world.y = touch->m_init_camera_rect.lt.y
        + (touch->m_init_camera_rect.rb.y - touch->m_init_camera_rect.lt.y) 
        * (evt_data->start_screen_pos[l_pos].y / camera->m_screen_size.y);

    if (ui_sprite_anim_camera_have_limit(camera)) {
        ui_sprite_anim_camera_touch_scale_adj_by_trace_x(
            touch, entity, camera, target_pos, target_scale, &lock_pos_in_screen, &lock_pos_in_world);
    }

    return 0;
}

void ui_sprite_anim_camera_touch_on_scale(void * ctx, ui_sprite_event_t evt) {
    ui_sprite_anim_camera_touch_t touch = ctx;
    ui_sprite_anim_module_t module = touch->m_module;
    UI_SPRITE_EVT_ANIM_CAMERA_TOUCH_SCALE const * evt_data = evt->data;
    ui_sprite_fsm_action_t action = ui_sprite_fsm_action_from_data(ctx);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(action);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_anim_camera_t camera = ui_sprite_anim_camera_find(world);
    UI_SPRITE_2D_PAIR target_pos;
    float target_scale;

    if (camera == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera-touch: on-scale: world no camera!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    if (evt_data->finger_count == 2) {
        switch(camera->m_trace_type) {
        case ui_sprite_anim_camera_trace_none:
            assert(0);
            //ui_sprite_anim_camera_adj_camera_in_limit_no_trace(camera, camera_pos, camera_scale);
            break;
        case ui_sprite_anim_camera_trace_by_x:
            if (ui_sprite_anim_camera_touch_on_scale_2_finter_trace_by_x(
                    touch, camera, entity,
                    &target_pos, &target_scale, evt_data)
                != 0)
            {
                return;
            }
            break;
        case ui_sprite_anim_camera_trace_by_y:
            assert(0);
            return;
        default:
            break;
        }
    }
    else {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera-touch: on-scale: not support scale by finger %d!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), evt_data->finger_count);
        return;
    }

    touch->m_state = ui_sprite_anim_camera_touch_state_move;

    ui_sprite_anim_camera_updator_set_max_speed(&touch->m_updator, evt_data->max_speed);

    ui_sprite_anim_camera_updator_set_camera(&touch->m_updator, camera, target_pos, target_scale);
}
