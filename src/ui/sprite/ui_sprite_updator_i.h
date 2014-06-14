#ifndef UI_SPRITE_WORLD_UPDATOR_I_H
#define UI_SPRITE_WORLD_UPDATOR_I_H
#include "cpe/pal/pal_queue.h"
#include "ui/sprite/ui_sprite_world.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_sprite_world_updator * ui_sprite_world_updator_t;
typedef TAILQ_HEAD(ui_sprite_world_updator_list, ui_sprite_world_updator) ui_sprite_world_updator_list_t;

struct ui_sprite_world_updator {
    ui_sprite_world_update_fun_t m_fun;
    void * m_ctx;
    TAILQ_ENTRY(ui_sprite_world_updator) m_next;
};

ui_sprite_world_updator_t ui_sprite_world_update_create(ui_sprite_world_t world, ui_sprite_world_update_fun_t fun, void * ctx);

void ui_sprite_world_update_free(ui_sprite_world_t world, ui_sprite_world_updator_t updator);
void ui_sprite_world_update_free_all(ui_sprite_world_t world);

#ifdef __cplusplus
}
#endif

#endif
