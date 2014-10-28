#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "render/model/ui_data_spine.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_mgr.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite/ui_sprite_component_meta.h"
#include "ui_sprite_spine_obj_i.h"
#include "ui_sprite_spine_env_i.h"

ui_sprite_spine_obj_t ui_sprite_spine_obj_create(ui_sprite_entity_t entity) {
    ui_sprite_component_t component = ui_sprite_component_create(entity, UI_SPRITE_SPINE_OBJ_NAME);
    return component ? ui_sprite_component_data(component) : NULL;
};

ui_sprite_spine_obj_t ui_sprite_spine_obj_find(ui_sprite_entity_t entity) {
    ui_sprite_component_t component = ui_sprite_component_find(entity, UI_SPRITE_SPINE_OBJ_NAME);
    return component ? ui_sprite_component_data(component) : NULL;
};

void ui_sprite_spine_obj_free(ui_sprite_spine_obj_t spine_obj) {
    ui_sprite_component_t component = ui_sprite_component_from_data(spine_obj);
    if (component) {
        ui_sprite_component_free(component);
    }
}

static int ui_sprite_spine_obj_do_init(ui_sprite_spine_module_t module, ui_sprite_spine_obj_t spine_obj, ui_sprite_entity_t entity) {
    bzero(spine_obj, sizeof(*spine_obj));
    return 0;
}

static int ui_sprite_spine_obj_init(ui_sprite_component_t component, void * ctx) {
    ui_sprite_spine_module_t module = ctx;
    ui_sprite_entity_t entity = ui_sprite_component_entity(component);
    ui_sprite_spine_obj_t spine_obj = ui_sprite_component_data(component);

    if (ui_sprite_spine_obj_do_init(module, spine_obj, entity) != 0) return -1;

    return 0;
}

static void ui_sprite_spine_obj_fini(ui_sprite_component_t component, void * ctx) {
    ui_sprite_spine_obj_t spine_obj = ui_sprite_component_data(component);

    assert(spine_obj->m_obj == NULL);
}

static int ui_sprite_spine_obj_copy(ui_sprite_component_t to, ui_sprite_component_t from, void * ctx) {
    ui_sprite_spine_module_t module = ctx;
    ui_sprite_entity_t entity = ui_sprite_component_entity(to);
    ui_sprite_spine_obj_t to_spine_obj = ui_sprite_component_data(to);
    ui_sprite_spine_obj_t from_spine_obj = ui_sprite_component_data(from);

    if (ui_sprite_spine_obj_do_init(module, to_spine_obj, entity) != 0) return -1;

    strncpy(to_spine_obj->m_obj_path, from_spine_obj->m_obj_path, sizeof(to_spine_obj->m_obj_path));

    return 0;
}

static void ui_sprite_spine_obj_exit(ui_sprite_component_t component, void * ctx) {
    ui_sprite_spine_obj_t spine_obj = ui_sprite_component_data(component);

    assert(spine_obj->m_obj);

    ui_spine_obj_free(spine_obj->m_obj);
    spine_obj->m_obj = NULL;
}

static int ui_sprite_spine_obj_enter(ui_sprite_component_t component, void * ctx) {
    ui_sprite_spine_module_t module = ctx;
    ui_sprite_spine_obj_t spine_obj = ui_sprite_component_data(component);
    ui_sprite_entity_t entity = ui_sprite_component_entity(component);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_spine_env_t env;
    ui_data_mgr_t data_mgr;
    ui_data_src_t src;
    ui_data_spine_t spine_data;
    spSkeletonData * skeleton_data;

    env = ui_sprite_spine_env_find(world);
    if (env == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine component init: no env!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    data_mgr = ui_data_mgr_find_nc(module->m_app, NULL);
    if (data_mgr == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine component init: ui data_mgr not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    src = ui_data_src_child_find_by_path(
        ui_data_mgr_src_root(data_mgr),
        spine_obj->m_obj_path, ui_data_src_type_spine);
    if (src == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine component init: src %s not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), spine_obj->m_obj_path);
        return -1;
    }

    spine_data = ui_data_src_product(src);
    if (spine_data == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine component init: src %s not loaded!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), spine_obj->m_obj_path);
        return -1;
    }

    skeleton_data = ui_data_spine_skeleton_data(spine_data);
    if (skeleton_data == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine component init: skeleton data %s not loaded!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), spine_obj->m_obj_path);
        return -1;
    }

    assert(spine_obj->m_obj == NULL);

    spine_obj->m_obj = ui_spine_obj_create_with_data(env->m_obj_mgr, skeleton_data, 0);
    if (spine_obj->m_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine component init: create spine obj fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    ui_spine_obj_set_debug_slots(spine_obj->m_obj, spine_obj->m_debug_slots);
    ui_spine_obj_set_debug_bones(spine_obj->m_obj, spine_obj->m_debug_bones);

    if (ui_sprite_component_start_update(component) != 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine component init: start update fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        ui_spine_obj_free(spine_obj->m_obj);
        spine_obj->m_obj = NULL;
        return -1;
    }

    return 0;
}

void ui_sprite_spine_obj_update(ui_sprite_component_t component, void * ctx, float delta_s) {
    ui_sprite_spine_obj_t spine_obj = ui_sprite_component_data(component);
    ui_spine_obj_update(spine_obj->m_obj, delta_s);
}

void ui_sprite_spine_obj_set_obj_path(ui_sprite_spine_obj_t spine_obj, const char * path) {
    strncpy(spine_obj->m_obj_path, path, sizeof(spine_obj->m_obj_path));
}

uint8_t ui_sprite_spine_obj_debug_slots(ui_sprite_spine_obj_t obj) {
    return obj->m_debug_slots;
}

void ui_sprite_spine_obj_set_debug_slots(ui_sprite_spine_obj_t obj, uint8_t debug_slots) {
    obj->m_debug_slots = debug_slots;
    if (obj->m_obj) ui_spine_obj_set_debug_slots(obj->m_obj, debug_slots);
}

uint8_t ui_sprite_spine_obj_debug_bones(ui_sprite_spine_obj_t obj) {
    return obj->m_debug_bones;
}

void ui_sprite_spine_obj_set_debug_bones(ui_sprite_spine_obj_t obj, uint8_t debug_bones) {
    obj->m_debug_bones = debug_bones;
    if (obj->m_obj) ui_spine_obj_set_debug_bones(obj->m_obj, debug_bones);
}

ui_spine_obj_t ui_sprite_spine_obj_data(ui_sprite_spine_obj_t spine_obj) {
    return spine_obj->m_obj;
}

int ui_sprite_spine_obj_regist(ui_sprite_spine_module_t module) {
    ui_sprite_component_meta_t meta;

    meta = ui_sprite_component_meta_create(
        module->m_repo,
        UI_SPRITE_SPINE_OBJ_NAME, sizeof(struct ui_sprite_spine_obj));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s register: meta create fail",
            ui_sprite_spine_module_name(module), UI_SPRITE_SPINE_OBJ_NAME);
        return -1;
    }

    ui_sprite_component_meta_set_init_fun(meta, ui_sprite_spine_obj_init, module);
    ui_sprite_component_meta_set_copy_fun(meta, ui_sprite_spine_obj_copy, module);
    ui_sprite_component_meta_set_enter_fun(meta, ui_sprite_spine_obj_enter, module);
    ui_sprite_component_meta_set_exit_fun(meta, ui_sprite_spine_obj_exit, module);
    ui_sprite_component_meta_set_update_fun(meta, ui_sprite_spine_obj_update, module);
    ui_sprite_component_meta_set_free_fun(meta, ui_sprite_spine_obj_fini, module);

    return 0;
}

void ui_sprite_spine_obj_unregist(ui_sprite_spine_module_t module) {
    ui_sprite_component_meta_t meta;

    meta = ui_sprite_component_meta_find(module->m_repo, UI_SPRITE_SPINE_OBJ_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s unregister: meta not exist",
            ui_sprite_spine_module_name(module), UI_SPRITE_SPINE_OBJ_NAME);
        return;
    }

    ui_sprite_component_meta_free(meta);
}

const char * UI_SPRITE_SPINE_OBJ_NAME = "SpineObj";
