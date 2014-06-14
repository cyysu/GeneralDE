#include "cpe/cfg/cfg_read.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_anim/ui_sprite_anim_camera_shake.h"
#include "ui_sprite_cfg_loader_i.h"

ui_sprite_fsm_action_t ui_sprite_cfg_load_action_camera_shake(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_cfg_loader_t loader = ctx;
    ui_sprite_anim_camera_shake_t anim_camera_shake = ui_sprite_anim_camera_shake_create(fsm_state, name);

    if (anim_camera_shake == NULL) {
        CPE_ERROR(loader->m_em, "%s: create anim_camera_shake action: create fail!", ui_sprite_cfg_loader_name(loader));
        return NULL;
    }

    return ui_sprite_fsm_action_from_data(anim_camera_shake);
}

