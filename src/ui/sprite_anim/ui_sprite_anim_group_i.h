#ifndef UI_SPRITE_ANIM_GROUP_I_H
#define UI_SPRITE_ANIM_GROUP_I_H
#include "ui/sprite_anim/ui_sprite_anim_group.h"
#include "ui_sprite_anim_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_anim_group {
    ui_sprite_anim_sch_t m_anim_sch;
    const char * m_name;
    uint32_t m_group_id;
    uint8_t m_base_pos_of_entity;
    UI_SPRITE_2D_PAIR m_pos_adj;
    uint8_t m_accept_scale;
    uint8_t m_adj_accept_scale;
    TAILQ_ENTRY(ui_sprite_anim_group) m_next_for_sch;
};

ui_sprite_anim_group_t ui_sprite_anim_group_clone(ui_sprite_anim_sch_t anim_sch, ui_sprite_anim_group_t o);

int ui_sprite_anim_group_enter(ui_sprite_anim_group_t group);
void ui_sprite_anim_group_exit(ui_sprite_anim_group_t group);

void ui_sprite_anim_group_update(ui_sprite_anim_group_t group, ui_sprite_2d_transform_t transform);

#ifdef __cplusplus
}
#endif

#endif
