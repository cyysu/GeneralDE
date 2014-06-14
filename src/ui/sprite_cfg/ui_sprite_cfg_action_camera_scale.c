#include "cpe/cfg/cfg_read.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_anim/ui_sprite_anim_camera_scale.h"
#include "ui_sprite_cfg_loader_i.h"

ui_sprite_fsm_action_t ui_sprite_cfg_load_action_camera_scale(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_cfg_loader_t loader = ctx;
    ui_sprite_anim_camera_scale_t anim_camera_scale = ui_sprite_anim_camera_scale_create(fsm_state, name);

    if (anim_camera_scale == NULL) {
        CPE_ERROR(loader->m_em, "%s: create anim_camera_scale action: create fail!", ui_sprite_cfg_loader_name(loader));
        return NULL;
    }

    ui_sprite_anim_camera_scale_set_priority(
        anim_camera_scale,
        cfg_get_int8(cfg, "priority", ui_sprite_anim_camera_scale_priority(anim_camera_scale)));

    return ui_sprite_fsm_action_from_data(anim_camera_scale);
}

