#ifndef UI_SPRITE_CFG_VALUE_GENERATOR_LOAD_I_H
#define UI_SPRITE_CFG_VALUE_GENERATOR_LOAD_I_H
#include "ui/sprite_basic/ui_sprite_basic_types.h"
#include "ui_sprite_cfg_loader_i.h"

#ifdef __cplusplus
extern "C" {
#endif

int ui_sprite_cfg_value_generator_load(
    ui_sprite_cfg_loader_t loader,
    UI_SPRITE_BASIC_VALUE_GENEARTOR_DEF * def,
    cfg_t cfg);

#ifdef __cplusplus
}
#endif

#endif
