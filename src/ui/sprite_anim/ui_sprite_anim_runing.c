#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui_sprite_anim_runing_i.h"
#include "ui_sprite_anim_sch_i.h"
#include "ui_sprite_anim_group_i.h"
#include "ui_sprite_anim_backend_i.h"
#include "ui_sprite_anim_module_i.h"

ui_sprite_anim_runing_t
ui_sprite_anim_runing_create(ui_sprite_anim_sch_t anim_sch, ui_sprite_anim_group_t group, const char * res) {
    ui_sprite_anim_backend_t backend = anim_sch->m_backend;
    ui_sprite_anim_module_t module = backend->m_module;
    ui_sprite_entity_t entity = ui_sprite_component_entity(ui_sprite_component_from_data(anim_sch));
    ui_sprite_anim_runing_t anim_runing;

    anim_runing = mem_alloc(module->m_alloc, sizeof(struct ui_sprite_anim_runing));
    if (anim_runing == NULL) {
        CPE_ERROR(module->m_em, "crate anim runing %s: alloc fail!", res);
        return NULL;
    }

    anim_runing->m_anim_sch = anim_sch;

    anim_runing->m_anim_id = 
        anim_sch->m_backend->m_def.m_start_fun(
            anim_sch->m_backend->m_def.m_ctx, group->m_group_id, res, 1, -1, -1);
    if (anim_runing->m_anim_id == UI_SPRITE_INVALID_ANIM_ID) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): start anim %s: start fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), res);
        mem_free(module->m_alloc, anim_runing);
        return NULL;
    }

    TAILQ_INSERT_TAIL(&backend->m_runing_anims, anim_runing, m_next_for_backend);
    TAILQ_INSERT_TAIL(&anim_sch->m_runings, anim_runing, m_next_for_sch);

    return anim_runing;
}

void ui_sprite_anim_runing_free(ui_sprite_anim_runing_t anim_runing) {
    ui_sprite_anim_sch_t anim_sch = anim_runing->m_anim_sch;
    ui_sprite_anim_backend_t backend = anim_sch->m_backend;
    ui_sprite_anim_module_t module = backend->m_module;

    backend->m_def.m_stop_fun(backend->m_def.m_ctx, anim_runing->m_anim_id);

    TAILQ_REMOVE(&anim_sch->m_runings, anim_runing, m_next_for_sch);
    TAILQ_REMOVE(&backend->m_runing_anims, anim_runing, m_next_for_backend);

    mem_free(module->m_alloc, anim_runing);
}

ui_sprite_anim_runing_t ui_sprite_anim_runing_find_by_id(ui_sprite_anim_sch_t anim_sch, int32_t anim_id) {
    ui_sprite_anim_runing_t anim_runing;

    TAILQ_FOREACH(anim_runing, &anim_sch->m_runings, m_next_for_sch) {
        if (anim_runing->m_anim_id == anim_id) return anim_runing;
    }

    return NULL;
}

