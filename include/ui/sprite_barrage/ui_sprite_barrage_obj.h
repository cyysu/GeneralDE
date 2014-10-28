#ifndef UI_SPRITE_BARRAGE_OBJ_H
#define UI_SPRITE_BARRAGE_OBJ_H
#include "cpe/cfg/cfg_types.h"
#include "ui_sprite_barrage_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_BARRAGE_OBJ_NAME;

ui_sprite_barrage_obj_t ui_sprite_barrage_obj_create(ui_sprite_entity_t entity);
ui_sprite_barrage_obj_t ui_sprite_barrage_obj_find(ui_sprite_entity_t entity);
void ui_sprite_barrage_obj_free(ui_sprite_barrage_obj_t barrage_obj);

int ui_sprite_barrage_obj_crate_emitters(ui_sprite_barrage_obj_t barrage_obj, cfg_t cfg);

void ui_sprite_barrage_obj_set_emitters_is_enable(ui_sprite_barrage_obj_t barrage_obj, const char * group_name, uint8_t is_enable);

#ifdef __cplusplus
}
#endif

#endif
