#ifndef UI_SPRITE_WORLD_H
#define UI_SPRITE_WORLD_H
#include "cpe/tl/tl_types.h"
#include "cpe/timer/timer_types.h"
#include "ui_sprite_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*ui_sprite_world_update_fun_t)(ui_sprite_world_t world, void * ctx, float delta_s);

ui_sprite_world_t ui_sprite_world_create(ui_sprite_repository_t repo);
void ui_sprite_world_free(ui_sprite_world_t world);

gd_app_context_t ui_sprite_world_app(ui_sprite_world_t world);
ui_sprite_repository_t ui_sprite_world_repository(ui_sprite_world_t world);
tl_manage_t ui_sprite_world_tl_mgr(ui_sprite_world_t world);
cpe_timer_mgr_t ui_sprite_world_timer_mgr(ui_sprite_world_t world);

ui_sprite_event_handler_t ui_sprite_world_add_event_handler(
    ui_sprite_world_t world, const char * event_name, ui_sprite_event_process_fun_t fun, void * ctx);

void ui_sprite_world_clear_event_handler_by_ctx(ui_sprite_world_t world, void * ctx);

uint8_t ui_sprite_world_is_tick_start(ui_sprite_world_t world);
int ui_sprite_world_start_tick(ui_sprite_world_t world);
void ui_sprite_world_stop_tick(ui_sprite_world_t world);

int ui_sprite_world_add_updator(ui_sprite_world_t world, ui_sprite_world_update_fun_t fun, void * ctx);
void ui_sprite_world_remove_updator(ui_sprite_world_t world, void * ctx);

tl_time_t ui_sprite_world_time(ui_sprite_world_t world);

ui_sprite_entity_it_t ui_sprite_world_entities(mem_allocrator_t alloc, ui_sprite_world_t world);

/*send event operations*/
void ui_sprite_world_send_event(
    ui_sprite_world_t world,
    LPDRMETA meta, void const * data, size_t size);

void ui_sprite_world_send_event_to(
    ui_sprite_world_t world, const char * targets,
    LPDRMETA meta, void const * data, size_t size);

#ifdef __cplusplus
}
#endif

#endif
