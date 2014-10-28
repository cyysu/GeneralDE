#include <assert.h>
#include "cpe/utils/math_ex.h"
#include "ui/sprite/ui_sprite_world_res.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui_sprite_anim_camera_i.h"
#include "ui_sprite_anim_module_i.h"
#include "ui_sprite_anim_backend_i.h"

static void ui_sprite_anim_camera_clear(ui_sprite_world_res_t world_res, void * ctx);

ui_sprite_anim_camera_t
ui_sprite_anim_camera_create(ui_sprite_anim_module_t module, ui_sprite_world_t world) {
    ui_sprite_anim_camera_t camera;
    ui_sprite_anim_backend_t backend;
    ui_sprite_world_res_t world_res = ui_sprite_world_res_create(world, UI_SPRITE_ANIM_CAMERA_TYPE_NAME, sizeof(struct ui_sprite_anim_camera));

    camera = ui_sprite_world_res_data(world_res);

    camera->m_module = module;
    camera->m_camera_base_pos.x = 0.0f;
    camera->m_camera_base_pos.y = 0.0f;
    camera->m_camera_base_scale.x = 1.0f;
    camera->m_camera_base_scale.y = 1.0f;

    camera->m_camera_pos.x = 0.0f;
    camera->m_camera_pos.y = 0.0f;
    camera->m_camera_scale = 1.0f;
    camera->m_camera_scale_pair.x = 1.0f;
    camera->m_camera_scale_pair.y = 1.0f;

    if ((backend = ui_sprite_anim_backend_find(world)) && backend->m_def.m_screen_size_fun) {
        camera->m_screen_size = backend->m_def.m_screen_size_fun(backend->m_def.m_ctx);
    }
    else {
        camera->m_screen_size.x = 960;
        camera->m_screen_size.y = 640;
    }

    camera->m_limit_lt.x = 0.0f;
    camera->m_limit_lt.y = 0.0f;
    camera->m_limit_rb.x = 0.0f;
    camera->m_limit_rb.y = 0.0f;

    camera->m_max_op_id = 0;
    camera->m_curent_op_id = 0;

    camera->m_trace_type = ui_sprite_anim_camera_trace_none;

    ui_sprite_world_res_set_free_fun(world_res, ui_sprite_anim_camera_clear, module);

    return camera;
}

static void ui_sprite_anim_camera_clear(ui_sprite_world_res_t world_res, void * ctx) {
}

void ui_sprite_anim_camera_free(ui_sprite_anim_camera_t camera) {
    ui_sprite_world_res_t world_res = ui_sprite_world_res_from_data(camera);

    ui_sprite_world_res_free(world_res);
}

ui_sprite_anim_camera_t ui_sprite_anim_camera_find(ui_sprite_world_t world) {
    ui_sprite_world_res_t world_res = ui_sprite_world_res_find(world, UI_SPRITE_ANIM_CAMERA_TYPE_NAME);
    return world_res ? ui_sprite_world_res_data(world_res) : NULL;
}

UI_SPRITE_2D_PAIR ui_sprite_anim_camera_screen_size(ui_sprite_anim_camera_t camera) {
    return camera->m_screen_size;
}

void ui_sprite_anim_camera_set_screen_size(ui_sprite_anim_camera_t camera, UI_SPRITE_2D_PAIR screen_size) {
    assert(screen_size.x > 0.0f);
    assert(screen_size.y > 0.0f);

    camera->m_screen_size = screen_size;
}

void ui_sprite_anim_camera_set_base_scale(ui_sprite_anim_camera_t camera, UI_SPRITE_2D_PAIR const * scale) {
    camera->m_camera_base_scale = *scale;

    ui_sprite_anim_camera_set_pos_and_scale(camera, camera->m_camera_pos, camera->m_camera_scale);
}

UI_SPRITE_2D_PAIR ui_sprite_anim_camera_base_scale(ui_sprite_anim_camera_t camera) {
    return camera->m_camera_base_scale;
}

void ui_sprite_anim_camera_set_base_pos(ui_sprite_anim_camera_t camera, UI_SPRITE_2D_PAIR const * pos) {
    camera->m_camera_base_pos = *pos;
}

UI_SPRITE_2D_PAIR ui_sprite_anim_camera_base_pos(ui_sprite_anim_camera_t camera) {
    return camera->m_camera_base_pos;
}

UI_SPRITE_2D_PAIR ui_sprite_anim_camera_limit_lt(ui_sprite_anim_camera_t camera) {
    return camera->m_limit_lt;
}

UI_SPRITE_2D_PAIR ui_sprite_anim_camera_limit_rb(ui_sprite_anim_camera_t camera) {
    return camera->m_limit_rb;
}

static float ui_sprite_anim_camera_scale_max_for_limit(ui_sprite_anim_camera_t camera) {
    float scale_max_x;
    float scale_max_y;

    scale_max_x = (camera->m_limit_rb.x - camera->m_limit_lt.x) / camera->m_screen_size.x;
    scale_max_y = (camera->m_limit_rb.y - camera->m_limit_lt.y) / camera->m_screen_size.y;

    return scale_max_x < scale_max_y ? scale_max_x : scale_max_y;
}

int ui_sprite_anim_camera_set_limit(ui_sprite_anim_camera_t camera, UI_SPRITE_2D_PAIR limit_lt, UI_SPRITE_2D_PAIR limit_rb) {
    float scale_max;

    if (limit_lt.x >= limit_rb.x || limit_lt.y >= limit_rb.y) {
        CPE_ERROR(
            camera->m_module->m_em, "camera set limit: lt=(%f,%f), rb=(%f,%f) is error!",
            limit_lt.x, limit_lt.y, limit_rb.x, limit_rb.y);
        return -1;
    }

    camera->m_limit_lt = limit_lt;
    camera->m_limit_rb = limit_rb;

    scale_max = ui_sprite_anim_camera_scale_max_for_limit(camera);
    if (scale_max < camera->m_scale_max) camera->m_scale_max = scale_max;

    return 0;
}

int8_t ui_sprite_anim_camera_have_limit(ui_sprite_anim_camera_t camera) {
    return camera->m_limit_lt.x < camera->m_limit_rb.x;
}

int ui_sprite_anim_camera_set_scale_range(ui_sprite_anim_camera_t camera, float scale_min, float scale_max) {
    if (scale_min > 0.0f && scale_max > 0.0f) {
        if (scale_min >= scale_max) {
            CPE_ERROR(camera->m_module->m_em, "camera set scale range: %f ~ %f is error!", scale_min, scale_max);
            return -1;
        }
    }

    if (ui_sprite_anim_camera_have_limit(camera)) {
        float limit_max_scale = ui_sprite_anim_camera_scale_max_for_limit(camera);
        if (scale_max > limit_max_scale) {
            CPE_ERROR(
                camera->m_module->m_em, "camera set scale range: %f ~ %f is error, max scale for limit is %f!",
                scale_min, scale_max, limit_max_scale);
            return -1;
        }
    }

    camera->m_scale_min = scale_min;
    if (scale_max > 0.0f) camera->m_scale_max = scale_max;

    return 0;
}

float ui_sprite_anim_camera_scale_min(ui_sprite_anim_camera_t camera) {
    return camera->m_scale_min;
}

float ui_sprite_anim_camera_scale_max(ui_sprite_anim_camera_t camera) {
    return camera->m_scale_max;
}

UI_SPRITE_2D_PAIR ui_sprite_anim_camera_pos(ui_sprite_anim_camera_t camera) {
    return camera->m_camera_pos;
}

float ui_sprite_anim_camera_scale(ui_sprite_anim_camera_t camera) {
    return camera->m_camera_scale;
}

UI_SPRITE_2D_PAIR ui_sprite_anim_camera_scale_pair(ui_sprite_anim_camera_t camera) {
    return camera->m_camera_scale_pair;
}

void ui_sprite_anim_camera_set_pos_and_scale(ui_sprite_anim_camera_t camera, UI_SPRITE_2D_PAIR pos, float scale) {
    ui_sprite_anim_backend_t backend;

    assert(scale > 0.0f);

    camera->m_camera_scale = scale;
    camera->m_camera_pos = pos;

    camera->m_camera_scale_pair.x = 1.0f / camera->m_camera_scale;
    camera->m_camera_scale_pair.y = 1.0f /camera->m_camera_scale;

    backend =
        ui_sprite_anim_backend_find(
            ui_sprite_world_res_world(
                ui_sprite_world_res_from_data(camera)));
    if (backend) {
        UI_SPRITE_2D_PAIR final_pos = camera->m_camera_pos;
        UI_SPRITE_2D_PAIR final_scale = camera->m_camera_scale_pair;

        final_pos.x += camera->m_camera_base_pos.x;
        final_pos.y += camera->m_camera_base_pos.y;

        final_scale.x *= camera->m_camera_base_scale.x;
        final_scale.y *= camera->m_camera_base_scale.y;

        backend->m_def.m_camera_update_fun(backend->m_def.m_ctx, final_pos, final_scale);
    }
}

void ui_sprite_anim_camera_rect(ui_sprite_anim_camera_t camera, UI_SPRITE_2D_RECT * rect) {
    rect->lt = camera->m_camera_pos;
    rect->rb = camera->m_camera_pos;
    rect->rb.x += camera->m_screen_size.x * camera->m_camera_scale;
    rect->rb.y += camera->m_screen_size.y * camera->m_camera_scale;
}

UI_SPRITE_2D_PAIR ui_sprite_anim_camera_center_pos(ui_sprite_anim_camera_t camera) {
    UI_SPRITE_2D_PAIR pos = camera->m_camera_pos;
    pos.x += (camera->m_screen_size.x / 2.0f) / camera->m_camera_scale_pair.x;
    pos.y += (camera->m_screen_size.y / 2.0f) / camera->m_camera_scale_pair.y;
    return pos;
}

UI_SPRITE_2D_PAIR ui_sprite_anim_camera_screen_to_world(ui_sprite_anim_camera_t camera, UI_SPRITE_2D_PAIR pos) {
    pos.x /= camera->m_camera_scale_pair.x * camera->m_camera_base_scale.x;
    pos.y /= camera->m_camera_scale_pair.y * camera->m_camera_base_scale.y;

    pos.x += camera->m_camera_pos.x + camera->m_camera_base_pos.x;
    pos.y += camera->m_camera_pos.y + camera->m_camera_base_pos.y;

    return pos;
}

UI_SPRITE_2D_PAIR ui_sprite_anim_camera_world_to_screen(ui_sprite_anim_camera_t camera, UI_SPRITE_2D_PAIR pos) {
    pos.x -= camera->m_camera_pos.x + camera->m_camera_base_pos.x;
    pos.y -= camera->m_camera_pos.y + camera->m_camera_base_pos.y;
    
    pos.x *= camera->m_camera_scale_pair.x * camera->m_camera_base_scale.x;
    pos.y *= camera->m_camera_scale_pair.y * camera->m_camera_base_scale.y;

    return pos;
}

UI_SPRITE_2D_PAIR
ui_sprite_anim_camera_calc_pos_from_pos_in_screen(
    ui_sprite_anim_camera_t camera, UI_SPRITE_2D_PAIR pos_in_world, UI_SPRITE_2D_PAIR pos_of_screen, float scale)
{
    pos_in_world.x -= (camera->m_screen_size.x * scale) * pos_of_screen.x;
    pos_in_world.y -= (camera->m_screen_size.y * scale) * pos_of_screen.y;

    return pos_in_world;
}

int ui_sprite_anim_camera_pos_of_entity(UI_SPRITE_2D_PAIR * pos, ui_sprite_world_t world, uint32_t entity_id, const char * entity_name, uint8_t pos_of_entity) {
    ui_sprite_entity_t entity;
    ui_sprite_2d_transform_t transform;

    if (entity_id) {
        entity = ui_sprite_entity_find_by_id(world, entity_id);
        if (entity == NULL) return -1;
    }
    else {
        entity = ui_sprite_entity_find_by_name(world, entity_name);
        if (entity == NULL) return -1;
    }

    transform = ui_sprite_2d_transform_find(entity);
    if (transform == NULL) return -1;

    *pos = ui_sprite_2d_transform_world_pos(transform, pos_of_entity, UI_SPRITE_2D_TRANSFORM_POS_ADJ_ALL);

    return 0;
}

int ui_sprite_anim_camera_set_trace(
    ui_sprite_anim_camera_t camera, enum ui_sprite_anim_camera_trace_type type,
    UI_SPRITE_2D_PAIR screen_pos, UI_SPRITE_2D_PAIR world_pos_a, UI_SPRITE_2D_PAIR world_pos_b)
{
    if (type != ui_sprite_anim_camera_trace_by_x && type != ui_sprite_anim_camera_trace_by_y) {
        CPE_ERROR(camera->m_module->m_em, "camera set trace: trace type %d not support", type);
        return -1;
    }

    if (type == ui_sprite_anim_camera_trace_by_x) {
        if ((world_pos_b.x - world_pos_a.x) < 1.0f) {
            CPE_ERROR(
                camera->m_module->m_em, "camera set trace: trace by x, diff in x too small, a=(%f,%f), b=(%f,%f)",
                world_pos_a.x, world_pos_a.y, world_pos_b.x, world_pos_b.y);
            return -1;
        }

        camera->m_trace_line.m_by_x.m_dy_dx = (world_pos_b.y - world_pos_a.y) / (world_pos_b.x - world_pos_a.x);
        camera->m_trace_line.m_by_x.m_base_y = screen_pos.y - screen_pos.x * camera->m_trace_line.m_by_x.m_dy_dx;

        /* printf( */
        /*     "set trace: input=(%f,%f)-(%f,%f), base: %f-%f, dy_dx=%f\n", */
        /*     world_pos_a.x, world_pos_a.y, world_pos_b.x, world_pos_b.y, */
        /*     camera->m_trace_line.m_by_x.m_base_y,  */
        /*     camera->m_trace_line.m_by_x.m_base_y + camera->m_trace_line.m_by_x.m_dy_dx, */
        /*     camera->m_trace_line.m_by_x.m_dy_dx); */

    }
    else if (type == ui_sprite_anim_camera_trace_by_y) {
        if ((world_pos_b.y - world_pos_a.y) < 1.0f) {
            CPE_ERROR(
                camera->m_module->m_em, "camera set trace: trace by y, diff in y too small, a=(%f,%f), b=(%f,%f)",
                world_pos_a.x, world_pos_a.y, world_pos_b.x, world_pos_b.y);
            return -1;
        }

        camera->m_trace_line.m_by_y.m_dx_dy = (world_pos_b.x - world_pos_a.x) / (world_pos_b.y - world_pos_a.y);
        camera->m_trace_line.m_by_y.m_base_x = screen_pos.x - screen_pos.y * camera->m_trace_line.m_by_y.m_dx_dy;
    }

    camera->m_trace_type = type;
    camera->m_trace_screen_pos = screen_pos;
    camera->m_trace_world_pos = world_pos_a;

    return 0;
}

void ui_sprite_anim_camera_remove_trace(ui_sprite_anim_camera_t camera) {
    camera->m_trace_type = ui_sprite_anim_camera_trace_none;
}

uint32_t ui_sprite_anim_camera_start_op(ui_sprite_anim_camera_t camera) {
    camera->m_curent_op_id = ++camera->m_max_op_id;
    return camera->m_curent_op_id;
}

void ui_sprite_anim_camera_stop_op(ui_sprite_anim_camera_t camera, uint32_t op_id) {
    if (camera->m_curent_op_id == op_id) {
        camera->m_curent_op_id = 0;
    }
}

float ui_sprite_anim_camera_trace_x2y(ui_sprite_anim_camera_t camera, float camera_x, float scale) {
    float line_pos_y;

    assert(camera->m_trace_type == ui_sprite_anim_camera_trace_by_x);

    line_pos_y = camera->m_trace_world_pos.y + (camera_x - camera->m_trace_world_pos.x) * camera->m_trace_line.m_by_x.m_dy_dx;

    return line_pos_y - camera->m_screen_size.y * scale * camera->m_trace_screen_pos.y;
}

float ui_sprite_anim_camera_trace_y2x(ui_sprite_anim_camera_t camera, float camera_y, float scale) {
    float line_pos_x;

    assert(camera->m_trace_type == ui_sprite_anim_camera_trace_by_y);

    line_pos_x = camera->m_trace_world_pos.x + (camera_y - camera->m_trace_world_pos.y) * camera->m_trace_line.m_by_y.m_dx_dy;

    return line_pos_x - camera->m_screen_size.x * scale * camera->m_trace_screen_pos.x;
}

float ui_sprite_anim_camera_screen_x2y_lock_x(ui_sprite_anim_camera_t camera, float screen_x, UI_SPRITE_2D_PAIR world_pos, float scale) {
    float lock_pos_x;
    float lock_pos_y;

    assert(camera->m_trace_type == ui_sprite_anim_camera_trace_by_x);

    lock_pos_x = world_pos.x - camera->m_screen_size.x * (screen_x - camera->m_trace_screen_pos.x) * scale;
    lock_pos_y = camera->m_trace_world_pos.y + (lock_pos_x - camera->m_trace_world_pos.x) * camera->m_trace_line.m_by_x.m_dy_dx;

    if (fabs(world_pos.x - lock_pos_x) < 0.49) {
        return camera->m_trace_screen_pos.y + (world_pos.y - lock_pos_y) / camera->m_screen_size.y / scale;
    }
    else {
        float d = (world_pos.y - lock_pos_y) / (world_pos.x - lock_pos_x);

        return camera->m_trace_screen_pos.y + (screen_x - camera->m_trace_screen_pos.x) * d;
    }
}

const char * UI_SPRITE_ANIM_CAMERA_TYPE_NAME = "AnimationCamera";
