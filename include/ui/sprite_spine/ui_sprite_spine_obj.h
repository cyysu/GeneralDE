#ifndef UI_SPRITE_SPINE_OBJ_H
#define UI_SPRITE_SPINE_OBJ_H
#include "ui_sprite_spine_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_SPINE_OBJ_NAME;

ui_sprite_spine_obj_t ui_sprite_spine_obj_create(ui_sprite_entity_t entity);
ui_sprite_spine_obj_t ui_sprite_spine_obj_find(ui_sprite_entity_t entity);
void ui_sprite_spine_obj_free(ui_sprite_spine_obj_t spine_obj);

ui_spine_obj_t ui_sprite_spine_obj_data(ui_sprite_spine_obj_t spine_obj);

void ui_sprite_spine_obj_set_obj_path(ui_sprite_spine_obj_t spine_obj, const char * path);

uint8_t ui_sprite_spine_obj_debug_slots(ui_sprite_spine_obj_t obj);
void ui_sprite_spine_obj_set_debug_slots(ui_sprite_spine_obj_t obj, uint8_t debug_slots);

uint8_t ui_sprite_spine_obj_debug_bones(ui_sprite_spine_obj_t obj);
void ui_sprite_spine_obj_set_debug_bones(ui_sprite_spine_obj_t obj, uint8_t debug_bones);

#ifdef __cplusplus
}
#endif

#endif
