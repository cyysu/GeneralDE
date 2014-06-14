#ifndef UI_SPRITE_TOUCH_MANAGER_H
#define UI_SPRITE_TOUCH_MANAGER_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash_string.h"
#include "gd/app/app_types.h"
#include "ui_sprite_touch_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_touch_mgr_t
ui_sprite_touch_mgr_create(
    gd_app_context_t app, ui_sprite_repository_t repo, ui_sprite_fsm_module_t fsm_module,
    mem_allocrator_t alloc, const char * name, error_monitor_t em);

void ui_sprite_touch_mgr_free(ui_sprite_touch_mgr_t repo);

ui_sprite_touch_mgr_t ui_sprite_touch_mgr_find(gd_app_context_t app, cpe_hash_string_t name);
ui_sprite_touch_mgr_t ui_sprite_touch_mgr_find_nc(gd_app_context_t app, const char * name);

gd_app_context_t ui_sprite_touch_mgr_app(ui_sprite_touch_mgr_t mgr);
const char * ui_sprite_touch_mgr_name(ui_sprite_touch_mgr_t mgr);

uint8_t ui_sprite_touch_mgr_state(ui_sprite_touch_mgr_t mgr);

int ui_sprite_touch_mgr_touch_begin(ui_sprite_touch_mgr_t mgr, int32_t touch_id, UI_SPRITE_2D_PAIR screen_pt);
int ui_sprite_touch_mgr_touch_move(ui_sprite_touch_mgr_t mgr, int32_t touch_id, UI_SPRITE_2D_PAIR screen_pt);
int ui_sprite_touch_mgr_touch_end(ui_sprite_touch_mgr_t mgr, int32_t touch_id, UI_SPRITE_2D_PAIR screen_pt);

#ifdef __cplusplus
}
#endif

#endif
