#ifndef UI_SPRITE_ANIM_DEF_H
#define UI_SPRITE_ANIM_DEF_H
#include "ui_sprite_anim_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_anim_def_it {
    ui_sprite_anim_def_t (*next)(struct ui_sprite_anim_def_it * it);
    char m_data[64];
};

ui_sprite_anim_def_t
ui_sprite_anim_def_create(
    ui_sprite_anim_sch_t anim_sch, const char * anim_name, const char * res, uint8_t auto_start);

void ui_sprite_anim_def_free(ui_sprite_anim_def_t def);

ui_sprite_anim_def_t
ui_sprite_anim_def_find(ui_sprite_anim_sch_t anim_sch, const char * name);

uint8_t ui_sprite_anim_def_auto_start(ui_sprite_anim_def_t anim_def);
const char * ui_sprite_anim_def_anim_name(ui_sprite_anim_def_t anim_def);
const char * ui_sprite_anim_def_anim_res(ui_sprite_anim_def_t anim_def);

void ui_sprite_anim_sch_defs(ui_sprite_anim_def_it_t it, ui_sprite_anim_sch_t anim_sch);

#define ui_sprite_anim_def_it_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif
