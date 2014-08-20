#include "cpe/cfg/cfg_read.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_anim/ui_sprite_anim_lock_on_screen.h"
#include "ui_sprite_cfg_loader_i.h"

ui_sprite_fsm_action_t ui_sprite_cfg_load_action_lock_on_screen(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_cfg_loader_t loader = ctx;
    ui_sprite_anim_lock_on_screen_t lock_on_screen = ui_sprite_anim_lock_on_screen_create(fsm_state, name);
    UI_SPRITE_2D_PAIR pos;
    const char * decorator;

    if (lock_on_screen == NULL) {
        CPE_ERROR(loader->m_em, "%s: create lock_on_screen action: create fail!", ui_sprite_cfg_loader_name(loader));
        return NULL;
    }

    pos =  ui_sprite_anim_lock_on_screen_pos(lock_on_screen);

    pos.x = cfg_get_float(cfg, "pos.x", pos.x);
    pos.y = cfg_get_float(cfg, "pos.y", pos.y);

    if ((decorator = cfg_get_string(cfg, "decorator", NULL))) {
        if (ui_sprite_anim_lock_on_screen_set_decorator(lock_on_screen, decorator) != 0) {
            CPE_ERROR(
                loader->m_em, "%s: create lock_on_screen action: set decorator %s fail!",
                ui_sprite_cfg_loader_name(loader), decorator);
            ui_sprite_anim_lock_on_screen_free(lock_on_screen);
            return NULL;
        }
    }

    ui_sprite_anim_lock_on_screen_set_pos(lock_on_screen, pos);

    ui_sprite_anim_lock_on_screen_set_scale(
        lock_on_screen, 
        cfg_get_float(cfg, "scale", ui_sprite_anim_lock_on_screen_scale(lock_on_screen)));

    ui_sprite_anim_lock_on_screen_set_max_speed(
        lock_on_screen,
        cfg_get_float(cfg, "max-speed", ui_sprite_anim_lock_on_screen_max_speed(lock_on_screen)));
    
    return ui_sprite_fsm_action_from_data(lock_on_screen);
}

