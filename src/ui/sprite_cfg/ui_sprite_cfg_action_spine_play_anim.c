#include "cpe/cfg/cfg_read.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_spine/ui_sprite_spine_play_anim.h"
#include "ui_sprite_cfg_loader_i.h"

ui_sprite_fsm_action_t ui_sprite_cfg_load_action_spine_play_anim(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_cfg_loader_t loader = ctx;
    ui_sprite_spine_play_anim_t spine_play_anim = ui_sprite_spine_play_anim_create(fsm_state, name);
    const char * anim_name;

    if (spine_play_anim == NULL) {
        CPE_ERROR(loader->m_em, "%s: create spine_play_anim action: create fail!", ui_sprite_cfg_loader_name(loader));
        return NULL;
    }

    anim_name = cfg_get_string(cfg, "anim-name", NULL);
    if (anim_name == NULL) {
        CPE_ERROR(
            loader->m_em, "%s: create spine_play_anim action: anim-name not configured",
            ui_sprite_cfg_loader_name(loader));
        ui_sprite_spine_play_anim_free(spine_play_anim);
        return NULL;
    }

    if (ui_sprite_spine_play_anim_set_name(spine_play_anim, anim_name) != 0) {
        CPE_ERROR(
            loader->m_em, "%s: create spine_play_anim action: set anim-name %s fail",
            ui_sprite_cfg_loader_name(loader), anim_name);
        ui_sprite_spine_play_anim_free(spine_play_anim);
        return NULL;
    }

    ui_sprite_spine_play_anim_set_is_loop(spine_play_anim, cfg_get_uint8(cfg, "is-loop", ui_sprite_spine_play_anim_is_loop(spine_play_anim)));

    return ui_sprite_fsm_action_from_data(spine_play_anim);
}

