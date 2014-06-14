#include "cpe/cfg/cfg_read.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_basic/ui_sprite_basic_send_event.h"
#include "ui_sprite_cfg_loader_i.h"

ui_sprite_fsm_action_t ui_sprite_cfg_load_action_send_event(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_cfg_loader_t loader = ctx;
    ui_sprite_basic_send_event_t send_event = ui_sprite_basic_send_event_create(fsm_state, name);

    if (send_event == NULL) {
        CPE_ERROR(loader->m_em, "%s: create send_event action: create fail!", ui_sprite_cfg_loader_name(loader));
        return NULL;
    }

    if (ui_sprite_basic_send_event_set_on_enter(send_event, cfg_get_string(cfg, "on-enter", NULL)) != 0) {
        CPE_ERROR(
            loader->m_em, "%s: create send_event action: set on-enter fail",
            ui_sprite_cfg_loader_name(loader));
        ui_sprite_basic_send_event_free(send_event);
        return NULL;
    }

    if (ui_sprite_basic_send_event_set_on_exit(send_event, cfg_get_string(cfg, "on-exit", NULL)) != 0) {
        CPE_ERROR(
            loader->m_em, "%s: create send_event action: set on-exit fail",
            ui_sprite_cfg_loader_name(loader));
        ui_sprite_basic_send_event_free(send_event);
        return NULL;
    }


    return ui_sprite_fsm_action_from_data(send_event);
}

