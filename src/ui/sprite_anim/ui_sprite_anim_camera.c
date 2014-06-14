#include <assert.h>
#include "ui/sprite/ui_sprite_world_res.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui_sprite_anim_camera_i.h"
#include "ui_sprite_anim_module_i.h"
#include "ui_sprite_anim_camera_restrict_i.h"
#include "ui_sprite_anim_camera_op_i.h"
#include "ui_sprite_anim_backend_i.h"

static void ui_sprite_anim_camera_clear(ui_sprite_world_res_t world_res, void * ctx);

ui_sprite_anim_camera_t
ui_sprite_anim_camera_create(ui_sprite_anim_module_t module, ui_sprite_world_t world) {
    ui_sprite_anim_camera_t camera;
    ui_sprite_anim_backend_t backend;
    ui_sprite_world_res_t world_res = ui_sprite_world_res_create(world, UI_SPRITE_ANIM_CAMERA_TYPE_NAME, sizeof(struct ui_sprite_anim_camera));

    camera = ui_sprite_world_res_data(world_res);

    camera->m_module = module;
    camera->m_updating = 0;
    camera->m_camera_pos.x = 0.0f;
    camera->m_camera_pos.y = 0.0f;
    camera->m_camera_scale = 1.0f;

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
    TAILQ_INIT(&camera->m_ops);

    TAILQ_INIT(&camera->m_restricts);

    ui_sprite_world_res_set_free_fun(world_res, ui_sprite_anim_camera_clear, module);

    return camera;
}

static void ui_sprite_anim_camera_clear(ui_sprite_world_res_t world_res, void * ctx) {
}

void ui_sprite_anim_camera_free(ui_sprite_anim_camera_t camera) {
    ui_sprite_world_res_t world_res = ui_sprite_world_res_from_data(camera);

    if (camera->m_updating) {
        ui_sprite_world_remove_updator(ui_sprite_world_res_world(world_res), camera);
        camera->m_updating = 0;
    }

    while(!TAILQ_EMPTY(&camera->m_restricts)) {
        ui_sprite_anim_camera_restrict_free(TAILQ_FIRST(&camera->m_restricts));
    }

    while(!TAILQ_EMPTY(&camera->m_ops)) {
        ui_sprite_anim_camera_op_free(TAILQ_FIRST(&camera->m_ops));
    }

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

UI_SPRITE_2D_PAIR ui_sprite_anim_camera_limit_lt(ui_sprite_anim_camera_t camera) {
    return camera->m_limit_lt;
}

UI_SPRITE_2D_PAIR ui_sprite_anim_camera_limit_rb(ui_sprite_anim_camera_t camera) {
    return camera->m_limit_rb;
}

void ui_sprite_anim_camera_set_limit(ui_sprite_anim_camera_t camera, UI_SPRITE_2D_PAIR limit_lt, UI_SPRITE_2D_PAIR limit_rb) {
    camera->m_limit_lt = limit_lt;
    camera->m_limit_rb = limit_rb;

    ui_sprite_anim_camera_set_pos_and_scale(camera, camera->m_camera_pos, camera->m_camera_scale);
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

    camera->m_camera_scale = scale;
    camera->m_camera_pos = pos;

    ui_sprite_anim_camera_restrict_adj(camera, &camera->m_camera_pos, &camera->m_camera_scale);

    camera->m_camera_scale_pair.x = 1.0f / camera->m_camera_scale;
    camera->m_camera_scale_pair.y = 1.0f /camera->m_camera_scale;

    backend =
        ui_sprite_anim_backend_find(
            ui_sprite_world_res_world(
                ui_sprite_world_res_from_data(camera)));
    if (backend) {
        backend->m_def.m_camera_update_fun(backend->m_def.m_ctx, camera->m_camera_pos, camera->m_camera_scale_pair);
    }
}

void ui_sprite_anim_camera_set_pos_and_scale_no_adj(ui_sprite_anim_camera_t camera, UI_SPRITE_2D_PAIR pos, float scale) {
    ui_sprite_anim_backend_t backend;

    camera->m_camera_scale = scale;
    camera->m_camera_pos = pos;

    camera->m_camera_scale_pair.x = 1.0f / camera->m_camera_scale;
    camera->m_camera_scale_pair.y = 1.0f /camera->m_camera_scale;

    backend =
        ui_sprite_anim_backend_find(
            ui_sprite_world_res_world(
                ui_sprite_world_res_from_data(camera)));
    if (backend) {
        backend->m_def.m_camera_update_fun(backend->m_def.m_ctx, camera->m_camera_pos, camera->m_camera_scale_pair);
    }
}

UI_SPRITE_2D_PAIR ui_sprite_anim_camera_center_pos(ui_sprite_anim_camera_t camera) {
    UI_SPRITE_2D_PAIR pos = camera->m_camera_pos;
    pos.x += (camera->m_screen_size.x / 2.0f) / camera->m_camera_scale_pair.x;
    pos.y += (camera->m_screen_size.y / 2.0f) / camera->m_camera_scale_pair.y;
    return pos;
}

UI_SPRITE_2D_PAIR ui_sprite_anim_camera_screen_to_world(ui_sprite_anim_camera_t camera, UI_SPRITE_2D_PAIR pos) {
    pos.x /= camera->m_camera_scale_pair.x;
    pos.y /= camera->m_camera_scale_pair.y;

    pos.x += camera->m_camera_pos.x;
    pos.y += camera->m_camera_pos.y;

    return pos;
}

UI_SPRITE_2D_PAIR ui_sprite_anim_camera_world_to_screen(ui_sprite_anim_camera_t camera, UI_SPRITE_2D_PAIR pos) {
    pos.x -= camera->m_camera_pos.x;
    pos.y -= camera->m_camera_pos.y;
    
    pos.x *= camera->m_camera_scale_pair.x;
    pos.y *= camera->m_camera_scale_pair.y;

    return pos;
}

static void ui_sprite_anim_camera_update_fun(ui_sprite_world_t world, void * ctx, float delta_s) {
    ui_sprite_anim_camera_t camera = ctx;
    ui_sprite_anim_module_t module = camera->m_module;
    ui_sprite_anim_camera_op_t op;

    op = TAILQ_FIRST(&camera->m_ops);

    if (op && !op->m_is_done) {
        switch(op->m_op.type) {
        case UI_SPRITE_ANIM_CAMERA_OP_TYPE_MOVE_TO_TARGET:
            ui_sprite_anim_camera_op_update_move_to_target(op, delta_s);
            break;
        case UI_SPRITE_ANIM_CAMERA_OP_TYPE_MOVE_BY_SPEED:
            ui_sprite_anim_camera_op_update_move_by_speed(op, delta_s);
            break;
        default:
            CPE_ERROR(
                module->m_em, "%s: camera update: top op %d type %d is unknown!",
                ui_sprite_anim_module_name(module), op->m_op_id, op->m_op.type);
            ui_sprite_anim_camera_op_free(op);
            break;
        }

        if (op->m_is_done) {
            switch(op->m_complete_policy) {
            case ui_sprite_anim_camera_op_complete_remove:
                ui_sprite_anim_camera_op_free(op);
                break;
            case ui_sprite_anim_camera_op_complete_keep:
                break;
            default:
                CPE_ERROR(
                    module->m_em, "%s: camera update: top op %d complete policy %d is unknown!",
                    ui_sprite_anim_module_name(module), op->m_op_id, op->m_complete_policy);
                ui_sprite_anim_camera_op_free(op);
                break;
            }
        }
    }

    ui_sprite_anim_camera_sync_update(camera);
}

void ui_sprite_anim_camera_sync_update(ui_sprite_anim_camera_t camera) {
    uint8_t need_update;

    need_update = 
        TAILQ_EMPTY(&camera->m_ops)
        ? 0
        : (TAILQ_FIRST(&camera->m_ops)->m_is_done
           ? 0
           : 1)
        ;

    if (need_update != camera->m_updating) {
        ui_sprite_anim_module_t module = camera->m_module;
        ui_sprite_world_t world = ui_sprite_world_res_world(ui_sprite_world_res_from_data(camera));

        if (camera->m_updating) {
            assert(need_update == 0);
            ui_sprite_world_remove_updator(world, camera);
            camera->m_updating = 0;
        }
        else {
            assert(need_update == 1);

            if (ui_sprite_world_add_updator(world, ui_sprite_anim_camera_update_fun, camera) != 0) {
                CPE_ERROR(
                    module->m_em, "%s: camera sync update: add world updator fail!",
                    ui_sprite_anim_module_name(module));
                return;
            }
            else {
                camera->m_updating = 1;
            }
        }

    }
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

    *pos = ui_sprite_2d_transform_pos(transform, pos_of_entity);

    return 0;
}

const char * UI_SPRITE_ANIM_CAMERA_TYPE_NAME = "AnimationCamera";
