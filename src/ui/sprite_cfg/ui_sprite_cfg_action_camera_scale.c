#include "cpe/cfg/cfg_read.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_anim/ui_sprite_anim_camera_scale.h"
#include "ui_sprite_cfg_loader_i.h"

ui_sprite_fsm_action_t ui_sprite_cfg_load_action_camera_scale(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_cfg_loader_t loader = ctx;
    ui_sprite_anim_camera_scale_t anim_camera_scale = ui_sprite_anim_camera_scale_create(fsm_state, name);
    const char * decorator;

    if (anim_camera_scale == NULL) {
        CPE_ERROR(loader->m_em, "%s: create anim_camera_scale action: create fail!", ui_sprite_cfg_loader_name(loader));
        return NULL;
    }

    if ((decorator = cfg_get_string(cfg, "decorator", NULL))) {
        if (ui_sprite_anim_camera_scale_set_decorator(anim_camera_scale, decorator) != 0) {
            CPE_ERROR(loader->m_em, "%s: create anim_camera_scale action: create fail!", ui_sprite_cfg_loader_name(loader));
            ui_sprite_anim_camera_scale_free(anim_camera_scale);
            return NULL;
        }
    }

    return ui_sprite_fsm_action_from_data(anim_camera_scale);
}

