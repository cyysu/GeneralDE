#include "cpe/cfg/cfg_read.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_anim/ui_sprite_anim_camera_wait_stop.h"
#include "ui_sprite_cfg_loader_i.h"

ui_sprite_fsm_action_t ui_sprite_cfg_load_action_camera_wait_stop(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_cfg_loader_t loader = ctx;
    ui_sprite_anim_camera_wait_stop_t anim_camera_wait_stop = ui_sprite_anim_camera_wait_stop_create(fsm_state, name);

    if (anim_camera_wait_stop == NULL) {
        CPE_ERROR(loader->m_em, "%s: create anim_camera_wait_stop action: create fail!", ui_sprite_cfg_loader_name(loader));
        return NULL;
    }

    return ui_sprite_fsm_action_from_data(anim_camera_wait_stop);
}

