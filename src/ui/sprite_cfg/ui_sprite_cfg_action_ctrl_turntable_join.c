#include "cpe/cfg/cfg_read.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_ctrl/ui_sprite_ctrl_turntable_join.h"
#include "ui_sprite_cfg_loader_i.h"

ui_sprite_fsm_action_t ui_sprite_cfg_load_action_ctrl_turntable_join(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_cfg_loader_t loader = ctx;
    ui_sprite_ctrl_turntable_join_t ctrl = ui_sprite_ctrl_turntable_join_create(fsm_state, name);
    const char * turntable;

    turntable = cfg_get_string(cfg, "turntable", NULL);
    if (turntable == NULL) {
        CPE_ERROR(loader->m_em, "%s: create ctrl_turntable_join action: turntable not configured!", ui_sprite_cfg_loader_name(loader));
        return NULL;
    }

    if (ctrl == NULL) {
        CPE_ERROR(loader->m_em, "%s: create ctrl_turntable_join action: create fail!", ui_sprite_cfg_loader_name(loader));
        return NULL;
    }

    ui_sprite_ctrl_turntable_join_set_turntable(ctrl, turntable);

    return ui_sprite_fsm_action_from_data(ctrl);
}
