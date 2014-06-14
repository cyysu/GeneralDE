#ifndef UI_SPRITE_ANIM_TEMPLATE_BINDING_H
#define UI_SPRITE_ANIM_TEMPLATE_BINDING_H
#include "ui_sprite_anim_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_anim_template_binding_t
ui_sprite_anim_template_binding_create(
    ui_sprite_anim_template_t tpl, const char * control, const char * attr_name, const char * attr_value);

#ifdef __cplusplus
}
#endif

#endif
