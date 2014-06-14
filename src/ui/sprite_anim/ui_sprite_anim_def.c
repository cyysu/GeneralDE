#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui_sprite_anim_def_i.h"
#include "ui_sprite_anim_sch_i.h"
#include "ui_sprite_anim_backend_i.h"
#include "ui_sprite_anim_module_i.h"

ui_sprite_anim_def_t ui_sprite_anim_def_create(ui_sprite_anim_sch_t anim_sch, const char * anim_name, const char * res, uint8_t auto_start) {
    ui_sprite_anim_backend_t backend = anim_sch->m_backend;
    ui_sprite_anim_module_t module = backend->m_module;
    ui_sprite_entity_t entity = ui_sprite_component_entity(ui_sprite_component_from_data(anim_sch));
    ui_sprite_anim_def_t anim_def;
    size_t name_len = strlen(anim_name) + 1;
    size_t res_len = strlen(res) + 1;
    char * p;

    if (anim_name[0] == 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): add anim %s(%s): name is empty!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
            anim_name, res);
        return NULL;
    }

    if (ui_sprite_anim_def_find(anim_sch, anim_name) != NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): add anim %s(%s): name duplicate!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
            anim_name, res);
        return NULL;
    }

    anim_def = mem_alloc(module->m_alloc, sizeof(struct ui_sprite_anim_def) + name_len + res_len);
    if (anim_def == NULL) {
        CPE_ERROR(module->m_em, "crate anim def %s: alloc fail!", anim_name);
        return NULL;
    }

    p = (char *)(anim_def + 1);
    memcpy(p, anim_name, name_len);
    memcpy(p + name_len, res, res_len);

    anim_def->m_anim_sch = anim_sch;
    anim_def->m_auto_start = auto_start;
    anim_def->m_anim_name = p;
    anim_def->m_anim_res = p + name_len;

    TAILQ_INSERT_TAIL(&anim_sch->m_defs, anim_def, m_next_for_sch);

    return anim_def;
}

void ui_sprite_anim_def_free(ui_sprite_anim_def_t anim_def) {
    ui_sprite_anim_sch_t anim_sch = anim_def->m_anim_sch;
    ui_sprite_anim_backend_t backend = anim_sch->m_backend;
    ui_sprite_anim_module_t module = backend->m_module;

    TAILQ_REMOVE(&anim_sch->m_defs, anim_def, m_next_for_sch);
    mem_free(module->m_alloc, anim_def);
}

ui_sprite_anim_def_t ui_sprite_anim_def_find(ui_sprite_anim_sch_t anim_sch, const char * name) {
    ui_sprite_anim_def_t anim_def;

    TAILQ_FOREACH(anim_def, &anim_sch->m_defs, m_next_for_sch) {
        if (strcmp(anim_def->m_anim_name, name) == 0) return anim_def;
    }

    return NULL;
}

uint8_t ui_sprite_anim_def_auto_start(ui_sprite_anim_def_t anim_def) {
    return anim_def->m_auto_start;
}

const char * ui_sprite_anim_def_anim_name(ui_sprite_anim_def_t anim_def) {
    return anim_def->m_anim_name;
}

const char * ui_sprite_anim_def_anim_res(ui_sprite_anim_def_t anim_def) {
    return anim_def->m_anim_res;
}

static ui_sprite_anim_def_t ui_sprite_anim_sch_defs_it_next(struct ui_sprite_anim_def_it * it) {
    ui_sprite_anim_def_t * data = (ui_sprite_anim_def_t *)(it->m_data);
    ui_sprite_anim_def_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_sch);

    return r;
}

void ui_sprite_anim_sch_defs(ui_sprite_anim_def_it_t it, ui_sprite_anim_sch_t anim_sch) {
    *(ui_sprite_anim_def_t *)(it->m_data) = TAILQ_FIRST(&anim_sch->m_defs);
    it->next = ui_sprite_anim_sch_defs_it_next;
}

