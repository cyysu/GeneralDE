#ifndef UI_SPRITE_DATA_BUILD_I_H
#define UI_SPRITE_DATA_BUILD_I_H
#include "ui/sprite/ui_sprite_entity_attr.h"
#include "ui_sprite_entity_i.h"

#ifdef __cplusplus
extern "C" {
#endif

int ui_sprite_data_build(
    dr_data_entry_t to, char * arg_value,
    ui_sprite_world_t world, ui_sprite_entity_t entity, dr_data_source_t data_source);

#ifdef __cplusplus
}
#endif

#endif

