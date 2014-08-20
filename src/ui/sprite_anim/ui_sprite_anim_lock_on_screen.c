#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui_sprite_anim_lock_on_screen_i.h"
#include "ui_sprite_anim_camera_utils.h"

ui_sprite_anim_lock_on_screen_t ui_sprite_anim_lock_on_screen_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_ANIM_LOCK_ON_SCREEN_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_anim_lock_on_screen_free(ui_sprite_anim_lock_on_screen_t lock_on_screen) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(lock_on_screen);
    ui_sprite_fsm_action_free(fsm_action);
}

UI_SPRITE_2D_PAIR ui_sprite_anim_lock_on_screen_pos(ui_sprite_anim_lock_on_screen_t lock_on_screen) {
    return lock_on_screen->m_pos_on_screen;
}

void ui_sprite_anim_lock_on_screen_set_pos(ui_sprite_anim_lock_on_screen_t lock_on_screen, UI_SPRITE_2D_PAIR pos) {
    lock_on_screen->m_pos_on_screen = pos;
}

float ui_sprite_anim_lock_on_screen_scale(ui_sprite_anim_lock_on_screen_t lock_on_screen) {
    return lock_on_screen->m_scale;
}

void ui_sprite_anim_lock_on_screen_set_scale(ui_sprite_anim_lock_on_screen_t lock_on_screen, float scale) {
    lock_on_screen->m_scale = scale;
}

float ui_sprite_anim_lock_on_screen_max_speed(ui_sprite_anim_lock_on_screen_t lock_on_screen) {
    return lock_on_screen->m_max_speed;
}

void ui_sprite_anim_lock_on_screen_set_max_speed(ui_sprite_anim_lock_on_screen_t lock_on_screen, float max_speed) {
    lock_on_screen->m_max_speed = max_speed;
}

int ui_sprite_anim_lock_on_screen_set_decorator(ui_sprite_anim_lock_on_screen_t lock_on_screen, const char * decorator) {
    return ui_percent_decorator_setup(&lock_on_screen->m_decorator, decorator, lock_on_screen->m_module->m_em);
}

static int ui_sprite_anim_lock_on_screen_update_pos(
    ui_sprite_anim_lock_on_screen_t lock_on_screen, ui_sprite_2d_transform_t transform, ui_sprite_anim_camera_t camer)
{
    float scale = camer->m_camera_scale * lock_on_screen->m_scale;
    UI_SPRITE_2D_PAIR entity_pos;
    UI_SPRITE_2D_PAIR pos_on_screen;

    if (lock_on_screen->m_duration <= 0.0f || lock_on_screen->m_runing_time >= lock_on_screen->m_duration) {
        pos_on_screen = lock_on_screen->m_pos_on_screen;
    }
    else {
        float percent = lock_on_screen->m_runing_time / lock_on_screen->m_duration;
        percent = ui_percent_decorator_decorate(&lock_on_screen->m_decorator, percent);

        pos_on_screen.x =
            lock_on_screen->m_init_pos_on_screen.x
            + (lock_on_screen->m_pos_on_screen.x - lock_on_screen->m_init_pos_on_screen.x) * percent;

        pos_on_screen.y =
            lock_on_screen->m_init_pos_on_screen.y
            + (lock_on_screen->m_pos_on_screen.y - lock_on_screen->m_init_pos_on_screen.y) * percent;
    }

    entity_pos.x = camer->m_camera_pos.x + pos_on_screen.x * scale;
    entity_pos.y = camer->m_camera_pos.y + pos_on_screen.y * scale;
    
    ui_sprite_2d_transform_set_origin_pos(transform, entity_pos);
    ui_sprite_2d_transform_set_scale(transform, scale);

    return 0;
}

static int ui_sprite_anim_lock_on_screen_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_anim_lock_on_screen_t lock_on_screen = ui_sprite_fsm_action_data(fsm_action); 
    ui_sprite_anim_module_t module = ctx;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_anim_camera_t camera = ui_sprite_anim_camera_find(world);
    ui_sprite_2d_transform_t transform = ui_sprite_2d_transform_find(entity);

    if (transform == NULL) {
        CPE_ERROR(
            lock_on_screen->m_module->m_em, "entity %d(%s): lock_on_screen: no transform!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    if (camera == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): lock_on_screen: no camera!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        ui_sprite_fsm_action_stop_update(fsm_action);
        return -1;
    }

    if (lock_on_screen->m_max_speed > 0.0f) {
        lock_on_screen->m_init_pos_on_screen = 
            ui_sprite_anim_camera_world_to_screen(camera, ui_sprite_2d_transform_origin_pos(transform));
        lock_on_screen->m_duration =
            cpe_math_distance(
                lock_on_screen->m_init_pos_on_screen.x, lock_on_screen->m_init_pos_on_screen.y, 
                lock_on_screen->m_pos_on_screen.x, lock_on_screen->m_pos_on_screen.y)
            / lock_on_screen->m_max_speed;
    }
    else {
        lock_on_screen->m_duration = 0.0f;
        if (ui_sprite_anim_lock_on_screen_update_pos(lock_on_screen, transform, camera) != 0) return -1;
    }

    lock_on_screen->m_runing_time = 0.0f;
    ui_sprite_fsm_action_start_update(fsm_action);

    return 0;
}

static void ui_sprite_anim_lock_on_screen_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

static int ui_sprite_anim_lock_on_screen_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_anim_lock_on_screen_t lock_on_screen = ui_sprite_fsm_action_data(fsm_action);

    bzero(lock_on_screen, sizeof(*lock_on_screen));

    lock_on_screen->m_module = ctx;
    lock_on_screen->m_scale = 1.0f;

    return 0;
}

static void ui_sprite_anim_lock_on_screen_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_anim_lock_on_screen_t lock_on_screen = ui_sprite_fsm_action_data(fsm_action);

    assert(lock_on_screen->m_updator.m_curent_op_id == 0);
}

static int ui_sprite_anim_lock_on_screen_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_anim_lock_on_screen_t to_on_screen = ui_sprite_fsm_action_data(to);
    ui_sprite_anim_lock_on_screen_t from_on_screen = ui_sprite_fsm_action_data(from);

    if (ui_sprite_anim_lock_on_screen_init(to, ctx)) return -1;

    to_on_screen->m_pos_on_screen = from_on_screen->m_pos_on_screen;
    to_on_screen->m_scale = from_on_screen->m_scale;
    to_on_screen->m_max_speed = from_on_screen->m_max_speed;
    memcpy(&to_on_screen->m_decorator, &from_on_screen->m_decorator, sizeof(to_on_screen->m_decorator));

    return 0;
}

static void ui_sprite_anim_lock_on_screen_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta) {
    ui_sprite_anim_lock_on_screen_t lock_on_screen = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_anim_module_t module = ctx;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_anim_camera_t camera = ui_sprite_anim_camera_find(world);
    ui_sprite_2d_transform_t transform = ui_sprite_2d_transform_find(entity);

    if (transform == NULL) {
        CPE_ERROR(
            lock_on_screen->m_module->m_em, "entity %d(%s): lock_on_screen: no transform!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    if (camera == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): lock_on_screen: no camera!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }

    lock_on_screen->m_runing_time += delta;

    ui_sprite_anim_lock_on_screen_update_pos(lock_on_screen, transform, camera);
}

int ui_sprite_anim_lock_on_screen_regist(ui_sprite_anim_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(module->m_fsm_module, UI_SPRITE_ANIM_LOCK_ON_SCREEN_NAME, sizeof(struct ui_sprite_anim_lock_on_screen));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: anim lock_on_screen register: meta create fail",
            ui_sprite_anim_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_anim_lock_on_screen_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_anim_lock_on_screen_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_anim_lock_on_screen_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_anim_lock_on_screen_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_anim_lock_on_screen_clear, module);
	ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_anim_lock_on_screen_update, module);

    return 0;
}

void ui_sprite_anim_lock_on_screen_unregist(ui_sprite_anim_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_ANIM_LOCK_ON_SCREEN_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: anim lock_on_screen unregister: meta not exist",
            ui_sprite_anim_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);
}

const char * UI_SPRITE_ANIM_LOCK_ON_SCREEN_NAME = "lock-on-screen";

