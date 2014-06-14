#include <assert.h>
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui_sprite_anim_group_i.h"
#include "ui_sprite_anim_sch_i.h"
#include "ui_sprite_anim_backend_i.h"
#include "ui_sprite_anim_module_i.h"

ui_sprite_anim_group_t ui_sprite_anim_group_create(ui_sprite_anim_sch_t anim_sch, const char * name) {
    ui_sprite_anim_backend_t backend = anim_sch->m_backend;
    ui_sprite_anim_module_t module = backend->m_module;
    ui_sprite_anim_group_t anim_group;
    size_t name_len = strlen(name) + 1;
    char * p;

    anim_group = mem_alloc(module->m_alloc, sizeof(struct ui_sprite_anim_group) + name_len);
    if (anim_group == NULL) {
        CPE_ERROR(module->m_em, "crate anim group %s: alloc fail!", name);
        return NULL;
    }

    p = (char *)(anim_group + 1);
    memcpy(p, name, name_len);

    anim_group->m_anim_sch = anim_sch;
    anim_group->m_name = p;
    anim_group->m_group_id = UI_SPRITE_INVALID_ANIM_ID;
    anim_group->m_base_pos_of_entity = UI_SPRITE_2D_TRANSFORM_POS_ORIGIN;
    anim_group->m_pos_adj.x = 0.0f;
    anim_group->m_pos_adj.y = 0.0f;
    anim_group->m_accept_scale = 1;
    anim_group->m_adj_accept_scale = 1;

    TAILQ_INSERT_TAIL(&anim_sch->m_groups, anim_group, m_next_for_sch);

    return anim_group;
}

ui_sprite_anim_group_t ui_sprite_anim_group_clone(ui_sprite_anim_sch_t anim_sch, ui_sprite_anim_group_t o) {
    ui_sprite_anim_group_t group = ui_sprite_anim_group_create(anim_sch, o->m_name);

    group->m_base_pos_of_entity = o->m_base_pos_of_entity;
    group->m_pos_adj = o->m_pos_adj;
    group->m_accept_scale = o->m_accept_scale;
    group->m_adj_accept_scale = o->m_adj_accept_scale;

    return group;
}

void ui_sprite_anim_group_free(ui_sprite_anim_group_t anim_group) {
    ui_sprite_anim_sch_t anim_sch = anim_group->m_anim_sch;
    ui_sprite_anim_backend_t backend = anim_sch->m_backend;
    ui_sprite_anim_module_t module = backend->m_module;

    if (anim_group->m_group_id != UI_SPRITE_INVALID_ANIM_ID) {
        ui_sprite_anim_group_exit(anim_group);
    }

    TAILQ_REMOVE(&anim_sch->m_groups, anim_group, m_next_for_sch);

    mem_free(module->m_alloc, anim_group);
}

ui_sprite_anim_group_t ui_sprite_anim_group_find_by_name(ui_sprite_anim_sch_t anim_sch, const char * name) {
    ui_sprite_anim_group_t anim_group;

    TAILQ_FOREACH(anim_group, &anim_sch->m_groups, m_next_for_sch) {
        if (strcmp(anim_group->m_name, name) == 0) return anim_group;
    }

    return NULL;
}

int ui_sprite_anim_group_enter(ui_sprite_anim_group_t group) {
    ui_sprite_anim_sch_t anim_sch = group->m_anim_sch;
    ui_sprite_anim_backend_t backend = anim_sch->m_backend;
    ui_sprite_anim_module_t module = backend->m_module;
    ui_sprite_entity_t entity = ui_sprite_component_entity(ui_sprite_component_from_data(anim_sch));

    if (group->m_group_id != UI_SPRITE_INVALID_ANIM_ID) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): group %s: enter: already entered!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), group->m_name);
        return -1;
    }

    group->m_group_id = 
        anim_sch->m_backend->m_def.m_create_group_fun(
            anim_sch->m_backend->m_def.m_ctx, anim_sch->m_default_layer);
    if (group->m_group_id == UI_SPRITE_INVALID_ANIM_ID) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): group %s: enter: create group fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), group->m_name);
        return -1;
    }

    return 0;
}

void ui_sprite_anim_group_exit(ui_sprite_anim_group_t group) {
    ui_sprite_anim_sch_t anim_sch = group->m_anim_sch;

    if (group->m_group_id == UI_SPRITE_INVALID_ANIM_ID) return;

    anim_sch->m_backend->m_def.m_remove_group_fun(
        anim_sch->m_backend->m_def.m_ctx, group->m_group_id);

    group->m_group_id = UI_SPRITE_INVALID_ANIM_ID;
}

uint8_t ui_sprite_anim_group_base_pos(ui_sprite_anim_group_t group) {
    return group->m_base_pos_of_entity;
}

void ui_sprite_anim_group_set_base_pos(ui_sprite_anim_group_t group, uint8_t base_pos) {
    group->m_base_pos_of_entity = base_pos;
}

uint8_t ui_sprite_anim_group_accept_scale(ui_sprite_anim_group_t group) {
    return group->m_accept_scale;
}

void ui_sprite_anim_group_set_accept_scale(ui_sprite_anim_group_t group, uint8_t accept_scale) {
    group->m_accept_scale = accept_scale;
}

UI_SPRITE_2D_PAIR ui_sprite_anim_group_pos_adj(ui_sprite_anim_group_t group) {
    return group->m_pos_adj;
}

void ui_sprite_anim_group_set_pos_adj(ui_sprite_anim_group_t group, UI_SPRITE_2D_PAIR pos_adj) {
    group->m_pos_adj = pos_adj;
}

uint8_t ui_sprite_anim_group_adj_accept_scale(ui_sprite_anim_group_t group) {
    return group->m_adj_accept_scale;
}

void ui_sprite_anim_group_set_adj_accept_scale(ui_sprite_anim_group_t group, uint8_t adj_accept_scale) {
    group->m_adj_accept_scale = adj_accept_scale;
}

void ui_sprite_anim_group_update(ui_sprite_anim_group_t group, ui_sprite_2d_transform_t transform) {
    ui_sprite_anim_backend_t backend = group->m_anim_sch->m_backend;
    UI_SPRITE_2D_PAIR scale = ui_sprite_2d_transform_scale(transform);
    UI_SPRITE_2D_PAIR pos = ui_sprite_2d_transform_pos(transform, group->m_base_pos_of_entity);
    float angle = ui_sprite_2d_transform_angle(transform);

    if (group->m_adj_accept_scale) {
        pos.x += group->m_pos_adj.x * scale.x;
        pos.y += group->m_pos_adj.y * scale.y;
    }
    else {
        pos.x += group->m_pos_adj.x;
        pos.y += group->m_pos_adj.y;
    }

    if (!group->m_accept_scale) {
        scale.x = scale.y = 1.0f;
    }

    backend->m_def.m_pos_update_fun(backend->m_def.m_ctx, group->m_group_id, pos);
    backend->m_def.m_scale_update_fun(backend->m_def.m_ctx, group->m_group_id, scale);
    backend->m_def.m_angle_update_fun(backend->m_def.m_ctx, group->m_group_id, angle);
}
