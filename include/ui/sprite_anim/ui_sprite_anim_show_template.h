#ifndef UI_SPRITE_ANIM_ACTION_SHOW_TEMPLATE_H
#define UI_SPRITE_ANIM_ACTION_SHOW_TEMPLATE_H
#include "ui_sprite_anim_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_ANIM_SHOW_TEMPLATE_TYPE_NAME;

ui_sprite_anim_show_template_t ui_sprite_anim_show_template_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_anim_show_template_free(ui_sprite_anim_show_template_t show_template);

const char * ui_sprite_anim_show_template_template(ui_sprite_anim_show_template_t show_template);
void ui_sprite_anim_show_template_set_template(ui_sprite_anim_show_template_t show_template, const char * res);

#ifdef __cplusplus
}
#endif

#endif
