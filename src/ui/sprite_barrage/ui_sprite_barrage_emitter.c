#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_src.h"
#include "plugin/barrage/plugin_barrage_module.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui_sprite_barrage_emitter_i.h"
#include "ui_sprite_barrage_obj_i.h"
#include "ui_sprite_barrage_env_i.h"

ui_sprite_barrage_emitter_t
ui_sprite_barrage_emitter_create(
    ui_sprite_barrage_obj_t barrage_obj, const char * group, const char * res)
{
    ui_sprite_barrage_module_t module = barrage_obj->m_module;
	ui_sprite_barrage_emitter_t emitter;
	ui_sprite_entity_t entity = ui_sprite_component_entity(ui_sprite_component_from_data(barrage_obj));
    size_t group_len = strlen(group) + 1;
    ui_sprite_barrage_env_t env;
    ui_data_src_t emitter_src;

    emitter_src =
        ui_data_src_child_find_by_path(
        ui_data_mgr_src_root(plugin_barrage_module_data_mgr(module->m_barrage_module)),
        res, ui_data_src_type_bullet_emitter);
    if (emitter_src == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): BarrageObj(%s): create emitter: emitter %s not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), group, res);
        return NULL;
    }

    env = ui_sprite_barrage_env_find(ui_sprite_entity_world(entity));
    if (env == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): BarrageObj(%s): create emitter: env not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), group);
        return NULL;
    }
    
    
    emitter = mem_alloc(module->m_alloc, sizeof(struct ui_sprite_barrage_emitter) + group_len);
    if (emitter == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): BarrageObj(%s): create emitter: alloc fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), group);
        return NULL;
    }

    memcpy(emitter + 1, group, group_len);

    emitter->m_obj = barrage_obj;
    emitter->m_group = (const char *)(emitter + 1);
    emitter->m_pos_base = UI_SPRITE_2D_TRANSFORM_POS_ORIGIN;
    emitter->m_pos.x = -100.0f;
    emitter->m_pos.y = 0.0f;

    emitter->m_emitter = plugin_barrage_emitter_create(env->m_env, ui_data_src_product(emitter_src));
    if (emitter->m_emitter == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): BarrageObj(%s): create emitter: create barragte_emitter fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), group);
        mem_free(module->m_alloc, emitter);
        return NULL;
    }

    TAILQ_INSERT_TAIL(&barrage_obj->m_emitters, emitter, m_next_for_obj);

    return emitter;
}

void ui_sprite_barrage_emitter_free(ui_sprite_barrage_emitter_t emitter) {
    ui_sprite_barrage_obj_t obj = emitter->m_obj;
    ui_sprite_barrage_module_t module = obj->m_module;

    TAILQ_REMOVE(&obj->m_emitters, emitter, m_next_for_obj);

    plugin_barrage_emitter_free(emitter->m_emitter);

    mem_free(module->m_alloc, emitter);
}

void ui_sprite_barrage_emitter_free_group(ui_sprite_barrage_obj_t barrage_obj, const char * group) {
    ui_sprite_barrage_emitter_t emitter, next;

    for(emitter = TAILQ_FIRST(&barrage_obj->m_emitters); emitter; emitter = next) {
        next = TAILQ_NEXT(emitter, m_next_for_obj);

        if (strcmp(emitter->m_group, group) == 0) {
            ui_sprite_barrage_emitter_free(emitter);
        }
    }
}

void ui_sprite_barrage_emitter_free_all(ui_sprite_barrage_obj_t obj) {
    while(!TAILQ_EMPTY(&obj->m_emitters)) {
        ui_sprite_barrage_emitter_free(TAILQ_FIRST(&obj->m_emitters));
    }
}

const char * ui_sprite_barrage_emitter_type(ui_sprite_barrage_emitter_t emitter) {
    return plugin_barrage_emitter_type(emitter->m_emitter);
}

plugin_barrage_emitter_t ui_sprite_barrage_emitter_emitter(ui_sprite_barrage_emitter_t emitter) {
    return emitter->m_emitter;
}

void ui_sprite_barrage_emitter_set_transform(ui_sprite_barrage_emitter_t emitter, ui_sprite_2d_transform_t transform) {
    UI_SPRITE_2D_PAIR base_pos;
    BARRAGE_PAIR pos;

    base_pos = ui_sprite_2d_transform_world_pos(transform, emitter->m_pos_base, UI_SPRITE_2D_TRANSFORM_POS_ADJ_ALL);

    pos.x = base_pos.x + emitter->m_pos.x;
    pos.y = base_pos.y + emitter->m_pos.y;

    plugin_barrage_emitter_set_world_pos(emitter->m_emitter, &pos, ui_sprite_2d_transform_angle(transform));
}

void ui_sprite_barrage_emitter_set_pos(ui_sprite_barrage_emitter_t emitter, UI_SPRITE_2D_PAIR const pos) {
	emitter->m_pos.x = pos.x;
	emitter->m_pos.y = pos.y;
}
