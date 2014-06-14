#ifndef UI_SPRITE_ANIM_TEMPLATE_H
#define UI_SPRITE_ANIM_TEMPLATE_H
#include "ui_sprite_anim_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_anim_template_t ui_sprite_anim_template_create(
    ui_sprite_anim_sch_t anim_sch, const char * name, const char * group, const char * res);
ui_sprite_anim_template_t ui_sprite_anim_template_find(ui_sprite_anim_sch_t anim_sch, const char * name);
void ui_sprite_anim_template_free(ui_sprite_anim_template_t anim_template);

#ifdef __cplusplus
}
#endif

#endif
