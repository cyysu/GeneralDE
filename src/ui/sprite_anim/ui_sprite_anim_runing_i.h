#ifndef UI_SPRITE_ANIM_RUNING_I_H
#define UI_SPRITE_ANIM_RUNING_I_H
#include "ui/sprite_anim/ui_sprite_anim_sch.h"
#include "ui_sprite_anim_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_anim_runing {
    ui_sprite_anim_sch_t m_anim_sch;
    ui_sprite_anim_sch_t m_sch;
    uint32_t m_anim_id;

    TAILQ_ENTRY(ui_sprite_anim_runing) m_next_for_backend;
    TAILQ_ENTRY(ui_sprite_anim_runing) m_next_for_sch;
};

ui_sprite_anim_runing_t ui_sprite_anim_runing_create(ui_sprite_anim_sch_t anim_sch, ui_sprite_anim_group_t group, const char * res);
void ui_sprite_anim_runing_free(ui_sprite_anim_runing_t anim_runing);

ui_sprite_anim_runing_t ui_sprite_anim_runing_find_by_id(ui_sprite_anim_sch_t anim_sch, int32_t m_anim_id);

#ifdef __cplusplus
}
#endif

#endif
