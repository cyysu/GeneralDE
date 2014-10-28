#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_context.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_mgr.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite/ui_sprite_component_meta.h"
#include "protocol/ui/sprite_2d/ui_sprite_2d_data.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui_sprite_barrage_obj_i.h"
#include "ui_sprite_barrage_env_i.h"
#include "ui_sprite_barrage_emitter_i.h"

ui_sprite_barrage_obj_t ui_sprite_barrage_obj_create(ui_sprite_entity_t entity) {
    ui_sprite_component_t component = ui_sprite_component_create(entity, UI_SPRITE_BARRAGE_OBJ_NAME);
    return component ? ui_sprite_component_data(component) : NULL;
};

ui_sprite_barrage_obj_t ui_sprite_barrage_obj_find(ui_sprite_entity_t entity) {
    ui_sprite_component_t component = ui_sprite_component_find(entity, UI_SPRITE_BARRAGE_OBJ_NAME);
    return component ? ui_sprite_component_data(component) : NULL;
};

void ui_sprite_barrage_obj_free(ui_sprite_barrage_obj_t barrage_obj) {
    ui_sprite_component_t component = ui_sprite_component_from_data(barrage_obj);
    if (component) {
        ui_sprite_component_free(component);
    }
}

int ui_sprite_barrage_obj_crate_emitters(ui_sprite_barrage_obj_t barrage_obj, cfg_t cfg) {
    ui_sprite_barrage_module_t module = barrage_obj->m_module;
    ui_sprite_entity_t entity = ui_sprite_component_entity(ui_sprite_component_from_data(barrage_obj));
    struct cfg_it emitter_cfgs;
    cfg_t emitter_cfg;
	UI_SPRITE_2D_PAIR pos;

	if(cfg_type(cfg) == CPE_CFG_TYPE_STRING) {
        cfg_it_init(&emitter_cfgs, cfg_find_cfg(gd_app_cfg(module->m_app), cfg_as_string(cfg, NULL)));
    }
    else {
        cfg_it_init(&emitter_cfgs, cfg);
    }

    while((emitter_cfg = cfg_it_next(&emitter_cfgs))) {
        const char * group = cfg_get_string(emitter_cfg, "group", NULL);
        const char * use = cfg_get_string(emitter_cfg, "use", NULL);

        ui_sprite_barrage_emitter_t emitter;

        if (group == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): create emitters: group not configured!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            return -1;
        }

		if (use == NULL) {
			CPE_ERROR(
				module->m_em, "entity %d(%s): create emitters: use not configured!",
				ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
			return -1;
		}

        emitter = ui_sprite_barrage_emitter_create(barrage_obj, group, use);
        if (emitter == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): create emitters: create emitter %s(use=%s) fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), group, use);
            return -1;
        }

		pos.x = cfg_get_float(emitter_cfg, "pos.x", 0.0f);
		pos.y = cfg_get_float(emitter_cfg, "pos.y", 0.0f);
		ui_sprite_barrage_emitter_set_pos(emitter, pos);
    }

    return 0;
}

static int ui_sprite_barrage_obj_do_init(ui_sprite_barrage_module_t module, ui_sprite_barrage_obj_t obj, ui_sprite_entity_t entity) {
    bzero(obj, sizeof(*obj));

    obj->m_module = module;
    TAILQ_INIT(&obj->m_emitters);

    return 0;
}

static int ui_sprite_barrage_obj_init(ui_sprite_component_t component, void * ctx) {
    ui_sprite_barrage_module_t module = ctx;
    ui_sprite_entity_t entity = ui_sprite_component_entity(component);
    ui_sprite_barrage_obj_t barrage_obj = ui_sprite_component_data(component);

    if (ui_sprite_barrage_obj_do_init(module, barrage_obj, entity) != 0) return -1;

    return 0;
}

static void ui_sprite_barrage_obj_fini(ui_sprite_component_t component, void * ctx) {
    ui_sprite_barrage_obj_t barrage_obj = ui_sprite_component_data(component);

    assert(TAILQ_EMPTY(&barrage_obj->m_emitters));
}

static int ui_sprite_barrage_obj_copy(ui_sprite_component_t to, ui_sprite_component_t from, void * ctx) {
    ui_sprite_barrage_module_t module = ctx;
    ui_sprite_entity_t entity = ui_sprite_component_entity(to);
    ui_sprite_barrage_obj_t from_barrage_obj = ui_sprite_component_data(from);
    ui_sprite_barrage_obj_t to_barrage_obj = ui_sprite_component_data(to);
    ui_sprite_barrage_emitter_t from_emitter;

    if (ui_sprite_barrage_obj_do_init(module, to_barrage_obj, entity) != 0) return -1;

    TAILQ_FOREACH(from_emitter, &from_barrage_obj->m_emitters, m_next_for_obj) {
        ui_sprite_barrage_emitter_t emitter;

        emitter = ui_sprite_barrage_emitter_create(
            to_barrage_obj, from_emitter->m_group, ui_sprite_barrage_emitter_type(from_emitter));
        if (emitter == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): create emitters: create emitter %s(use=%s) fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                from_emitter->m_group, ui_sprite_barrage_emitter_type(from_emitter));
            return -1;
        }
    }

    return 0;
}

static void ui_sprite_barrage_obj_exit(ui_sprite_component_t component, void * ctx) {
    ui_sprite_barrage_obj_t barrage_obj = ui_sprite_component_data(component);

    ui_sprite_barrage_emitter_free_all(barrage_obj);
}

static void ui_sprite_barrage_obj_on_transform_update(void * ctx) {
    ui_sprite_barrage_emitter_t emitter;
    ui_sprite_barrage_obj_t barrage_obj = ctx;
    ui_sprite_entity_t entity = ui_sprite_component_entity(ui_sprite_component_from_data(barrage_obj));
    ui_sprite_2d_transform_t transform;

    if ((transform = ui_sprite_2d_transform_find(entity))) {
        TAILQ_FOREACH(emitter, &barrage_obj->m_emitters, m_next_for_obj) {
            ui_sprite_barrage_emitter_set_transform(emitter, transform);
        }
    }
}

static int ui_sprite_barrage_obj_enter(ui_sprite_component_t component, void * ctx) {
    ui_sprite_barrage_module_t module = ctx;
    ui_sprite_barrage_obj_t barrage_obj = ui_sprite_component_data(component);
    ui_sprite_entity_t entity = ui_sprite_component_entity(component);
    ui_sprite_2d_transform_t transform;
    ui_sprite_barrage_emitter_t emitter;

    if ((transform = ui_sprite_2d_transform_find(entity))) {
        TAILQ_FOREACH(emitter, &barrage_obj->m_emitters, m_next_for_obj) {
            ui_sprite_barrage_emitter_set_transform(emitter, transform);
        }

        if (ui_sprite_component_add_attr_monitor(
                component, "transform.pos,transform.angle",
                ui_sprite_barrage_obj_on_transform_update, barrage_obj)
            == NULL)
        {
            CPE_ERROR(
                module->m_em, "entity %d(%s): barrage obj enter: add attr montor fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            return -1;
        }
    }
    
    return 0;
}

static int ui_sprite_barrage_obj_load(void * ctx, ui_sprite_component_t comp, cfg_t cfg) {
    ui_sprite_barrage_module_t module = ctx;
    ui_sprite_entity_t entity = ui_sprite_component_entity(comp);
    ui_sprite_barrage_obj_t barrage_obj = ui_sprite_component_data(comp);
    cfg_t emitter_cfg;

    if ((emitter_cfg = cfg_find_cfg(cfg, "emitters"))) {
        if (ui_sprite_barrage_obj_crate_emitters(barrage_obj, emitter_cfg) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): barrage obj create: load emitters fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            return -1;
        }
    }

    return 0;
}

void ui_sprite_barrage_obj_set_emitters_is_enable(ui_sprite_barrage_obj_t barrage_obj, const char * group_name, uint8_t is_enable) {
    ui_sprite_barrage_emitter_t emitter;

    if (group_name == NULL) {
        TAILQ_FOREACH(emitter, &barrage_obj->m_emitters, m_next_for_obj) {
            plugin_barrage_emitter_set_is_enable(emitter->m_emitter, is_enable);
        }
    }
    else {
        TAILQ_FOREACH(emitter, &barrage_obj->m_emitters, m_next_for_obj) {
            if (strcmp(emitter->m_group, group_name) == 0) {
                plugin_barrage_emitter_set_is_enable(emitter->m_emitter, is_enable);
            }
        }
    }
}

int ui_sprite_barrage_obj_regist(ui_sprite_barrage_module_t module) {
    ui_sprite_component_meta_t meta;

    meta = ui_sprite_component_meta_create(
        module->m_repo,
        UI_SPRITE_BARRAGE_OBJ_NAME, sizeof(struct ui_sprite_barrage_obj));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s register: meta create fail",
            ui_sprite_barrage_module_name(module), UI_SPRITE_BARRAGE_OBJ_NAME);
        return -1;
    }

    ui_sprite_component_meta_set_init_fun(meta, ui_sprite_barrage_obj_init, module);
    ui_sprite_component_meta_set_copy_fun(meta, ui_sprite_barrage_obj_copy, module);
    ui_sprite_component_meta_set_enter_fun(meta, ui_sprite_barrage_obj_enter, module);
    ui_sprite_component_meta_set_exit_fun(meta, ui_sprite_barrage_obj_exit, module);
    ui_sprite_component_meta_set_free_fun(meta, ui_sprite_barrage_obj_fini, module);

    if (ui_sprite_cfg_loader_add_comp_loader(
            module->m_loader, UI_SPRITE_BARRAGE_OBJ_NAME, ui_sprite_barrage_obj_load, module)
        != 0)
    {
        CPE_ERROR(
            module->m_em, "%s: %s register: register loader fail",
            ui_sprite_barrage_module_name(module), UI_SPRITE_BARRAGE_OBJ_NAME);
        ui_sprite_component_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_barrage_obj_unregist(ui_sprite_barrage_module_t module) {
    ui_sprite_component_meta_t meta;

    if ((meta = ui_sprite_component_meta_find(module->m_repo, UI_SPRITE_BARRAGE_OBJ_NAME))) {
        ui_sprite_component_meta_free(meta);
    }
    else {
        CPE_ERROR(
            module->m_em, "%s: %s unregister: meta not exist",
            ui_sprite_barrage_module_name(module), UI_SPRITE_BARRAGE_OBJ_NAME);
    }

    if (ui_sprite_cfg_loader_remove_comp_loader(module->m_loader, UI_SPRITE_BARRAGE_OBJ_NAME) != 0) {
        CPE_ERROR(
            module->m_em, "%s: %s unregister: loader remove fail",
            ui_sprite_barrage_module_name(module), UI_SPRITE_BARRAGE_OBJ_NAME);
    }
}

const char * UI_SPRITE_BARRAGE_OBJ_NAME = "BarrageObj";
