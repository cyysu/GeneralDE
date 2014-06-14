#include "cpe/cfg/cfg_read.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_anim/ui_sprite_anim_show_animation.h"
#include "ui_sprite_cfg_loader_i.h"

ui_sprite_fsm_action_t ui_sprite_cfg_load_action_show_animation(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_cfg_loader_t loader = ctx;
    ui_sprite_anim_show_animation_t show_animation = ui_sprite_anim_show_animation_create(fsm_state, name);
    const char * animation_res;

    if (show_animation == NULL) {
        CPE_ERROR(loader->m_em, "%s: create show_animation action: create fail!", ui_sprite_cfg_loader_name(loader));
        return NULL;
    }

    if ((animation_res = cfg_get_string(cfg, "res", NULL)) == NULL) {
        CPE_ERROR(
            loader->m_em, "%s: create show_animation action: res not configured!",
            ui_sprite_cfg_loader_name(loader));
        ui_sprite_anim_show_animation_free(show_animation);
        return NULL;
    }
    ui_sprite_anim_show_animation_set_res(show_animation, animation_res);

    ui_sprite_anim_show_animation_set_group(show_animation, cfg_get_string(cfg, "group", ""));
    ui_sprite_anim_show_animation_set_loop(show_animation, cfg_get_uint8(cfg, "loop", 0));
    ui_sprite_anim_show_animation_set_start(show_animation, cfg_get_int32(cfg, "start", -1));
    ui_sprite_anim_show_animation_set_end(show_animation, cfg_get_int32(cfg, "end", -1));

    return ui_sprite_fsm_action_from_data(show_animation);
}

