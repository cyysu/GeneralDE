#ifndef UI_SPRITE_ANIM_SCH_H
#define UI_SPRITE_ANIM_SCH_H
#include "ui_sprite_anim_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_ANIM_SCH_NAME;

ui_sprite_anim_sch_t ui_sprite_anim_sch_create(ui_sprite_entity_t entity);
ui_sprite_anim_sch_t ui_sprite_anim_sch_find(ui_sprite_entity_t entity);
void ui_sprite_anim_sch_free(ui_sprite_anim_sch_t anim_sch);

uint32_t ui_sprite_anim_sch_start_anim(
    ui_sprite_anim_sch_t anim_sch, const char * group, const char * res, uint8_t is_loop, int32_t start, int32_t end);
void ui_sprite_anim_sch_stop_anim(ui_sprite_anim_sch_t anim_sch, uint32_t anim_id);
uint8_t ui_sprite_anim_sch_is_anim_runing(ui_sprite_anim_sch_t anim_sch, uint32_t anim_id);

void ui_sprite_anim_sch_set_default_layer(ui_sprite_anim_sch_t anim_sch, const char * layer_name);
int ui_sprite_anim_sch_add_animation(ui_sprite_anim_sch_t anim_sch, const char * anim_name, const char * res, uint8_t auto_start);
const char * ui_sprite_anim_sch_find_res(ui_sprite_anim_sch_t anim_sch, const char * name);

#ifdef __cplusplus
}
#endif

#endif
