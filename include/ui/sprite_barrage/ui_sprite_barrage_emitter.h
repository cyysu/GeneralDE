#ifndef UI_SPRITE_BARRAGE_EMITTER_H
#define UI_SPRITE_BARRAGE_EMITTER_H
#include "ui_sprite_barrage_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_barrage_emitter_t
ui_sprite_barrage_emitter_create(
    ui_sprite_barrage_obj_t barrage_obj, const char * group, const char * res);
void ui_sprite_barrage_emitter_free(ui_sprite_barrage_emitter_t emitter);
void ui_sprite_barrage_emitter_free_group(ui_sprite_barrage_obj_t barrage_obj, const char * group);
void ui_sprite_barrage_emitter_free_all(ui_sprite_barrage_obj_t barrage_obj);

const char * ui_sprite_barrage_emitter_type(ui_sprite_barrage_emitter_t emitter);
plugin_barrage_emitter_t ui_sprite_barrage_emitter_emitter(ui_sprite_barrage_emitter_t emitter);

void ui_sprite_barrage_emitter_set_transform(ui_sprite_barrage_emitter_t emitter, ui_sprite_2d_transform_t transform);
void ui_sprite_barrage_emitter_set_pos(ui_sprite_barrage_emitter_t emitter, UI_SPRITE_2D_PAIR const pos);

#ifdef __cplusplus
}
#endif

#endif
