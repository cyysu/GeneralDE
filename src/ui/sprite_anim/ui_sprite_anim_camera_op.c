#include "ui_sprite_anim_camera_op_i.h"

uint32_t ui_sprite_anim_camera_op_start(
    ui_sprite_anim_camera_t camera,
    UI_SPRITE_ANIM_CAMERA_OP const * op,
    int8_t priority,
    ui_sprite_anim_camera_op_suspend_policy_t suspend_policy,
    ui_sprite_anim_camera_op_complete_policy_t complete_policy)
{
    ui_sprite_anim_module_t module = camera->m_module;
    ui_sprite_anim_camera_op_t camera_op;
    uint32_t new_op_id;
    ui_sprite_anim_camera_op_t insert_after = NULL;


    new_op_id = camera->m_max_op_id + 1;

    camera_op = mem_alloc(module->m_alloc, sizeof(struct ui_sprite_anim_camera_op));
    if (camera_op == NULL) {
        CPE_ERROR(module->m_em, "camera: start op: alloc fail");
        return UI_SPRITE_INVALID_CAMERA_OP_ID;
    }

    if (!TAILQ_EMPTY(&camera->m_ops)) {
        ui_sprite_anim_camera_op_t check_op;

        TAILQ_FOREACH(check_op, &camera->m_ops, m_next_for_camera) {
            if (check_op->m_priority > priority) {
                insert_after = check_op;
            }
            else {
                break;
            }
        }

        if (insert_after) {
            if (suspend_policy == ui_sprite_anim_camera_op_suspend_remove) {
                mem_free(module->m_alloc, camera_op);
                return UI_SPRITE_INVALID_CAMERA_OP_ID;
            }
        }
        
        for(; check_op != TAILQ_END(&camera->m_ops);) {
            ui_sprite_anim_camera_op_t next_op = TAILQ_NEXT(check_op, m_next_for_camera);

            if (check_op->m_suspend_policy == ui_sprite_anim_camera_op_suspend_remove) {
                ui_sprite_anim_camera_op_free(check_op);
            }

            check_op = next_op;
        }
    }

    camera_op->m_camera = camera;
    camera_op->m_op_id = new_op_id;
    camera_op->m_is_done = 0;
    camera_op->m_op = *op;
    camera_op->m_priority = priority;
    camera_op->m_suspend_policy = suspend_policy;
    camera_op->m_complete_policy = complete_policy;

    camera->m_max_op_id++;
    if (insert_after) {
        TAILQ_INSERT_AFTER(&camera->m_ops, insert_after, camera_op, m_next_for_camera);
    }
    else {
        TAILQ_INSERT_HEAD(&camera->m_ops, camera_op, m_next_for_camera);
    }

    ui_sprite_anim_camera_sync_update(camera);

    return new_op_id;
}

uint32_t
ui_sprite_anim_camera_op_check_start(
    ui_sprite_anim_camera_t camera,
    uint32_t op_id,
    UI_SPRITE_ANIM_CAMERA_OP const * op_data,
    int8_t priority,
    ui_sprite_anim_camera_op_suspend_policy_t suspend_policy,
    ui_sprite_anim_camera_op_complete_policy_t complete_policy)
{
    ui_sprite_anim_camera_op_t op = 
        op_id == UI_SPRITE_INVALID_CAMERA_OP_ID
        ? NULL
        : ui_sprite_anim_camera_op_find(camera, op_id);

    if (op) {
        op->m_op = *op_data;
        op->m_priority = priority;
        op->m_suspend_policy = suspend_policy;
        op->m_complete_policy = complete_policy;

        if (op->m_is_done) {
            op->m_is_done = 0;
            ui_sprite_anim_camera_sync_update(camera);
        }

        return op_id;
    }
    else {
        return ui_sprite_anim_camera_op_start(camera, op_data, priority, suspend_policy, complete_policy);
    }
}

ui_sprite_anim_camera_op_t ui_sprite_anim_camera_op_find(ui_sprite_anim_camera_t camera, uint32_t op_id) {
    ui_sprite_anim_camera_op_t camera_op;

    TAILQ_FOREACH(camera_op, &camera->m_ops, m_next_for_camera) {
        if (camera_op->m_op_id == op_id) return camera_op; 
    }

    return NULL;
}

void ui_sprite_anim_camera_op_free(ui_sprite_anim_camera_op_t camera_op) {
    ui_sprite_anim_camera_t camera = camera_op->m_camera;

    TAILQ_REMOVE(&camera->m_ops, camera_op, m_next_for_camera);

    mem_free(camera->m_module->m_alloc, camera_op);
}

void ui_sprite_anim_camera_op_stop(ui_sprite_anim_camera_t camera, uint32_t op_id) {
    ui_sprite_anim_camera_op_t camera_op = ui_sprite_anim_camera_op_find(camera, op_id);
    if (camera_op) {
        ui_sprite_anim_camera_op_free(camera_op);
    }

    ui_sprite_anim_camera_sync_update(camera);
}

uint8_t ui_sprite_anim_camera_op_is_runing(ui_sprite_anim_camera_t camera, uint32_t op_id) {
    ui_sprite_anim_camera_op_t op = ui_sprite_anim_camera_op_find(camera, op_id);
    return op == NULL
        ? 0
        : (op->m_is_done ? 0 : 1);
}

uint32_t ui_sprite_anim_camera_op_curent(ui_sprite_anim_camera_t camera) {
    return TAILQ_EMPTY(&camera->m_ops) ? UI_SPRITE_INVALID_CAMERA_OP_ID : TAILQ_FIRST(&camera->m_ops)->m_op_id;
}
