#include "cpe/cfg/cfg_read.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_ctrl/ui_sprite_ctrl_turntable_touch.h"
#include "ui_sprite_cfg_loader_i.h"

ui_sprite_fsm_action_t ui_sprite_cfg_load_action_ctrl_turntable_touch(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_cfg_loader_t loader = ctx;
    ui_sprite_ctrl_turntable_touch_t turntable_touch = ui_sprite_ctrl_turntable_touch_create(fsm_state, name);
    const char * decorator;

    if (turntable_touch == NULL) {
        CPE_ERROR(loader->m_em, "%s: create ctrl_turntable_touch action: create fail!", ui_sprite_cfg_loader_name(loader));
        return NULL;
    }

    if ((decorator = cfg_get_string(cfg, "decorator", NULL))) {
        if (ui_sprite_ctrl_turntable_touch_set_decorator(turntable_touch, decorator) != 0) {
            CPE_ERROR(loader->m_em, "%s: create ctrl_turntable_touch action: create fail!", ui_sprite_cfg_loader_name(loader));
            ui_sprite_ctrl_turntable_touch_free(turntable_touch);
            return NULL;
        }
    }

    return ui_sprite_fsm_action_from_data(turntable_touch);
}
