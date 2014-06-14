#include "cpe/cfg/cfg_read.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_ctrl/ui_sprite_ctrl_circle.h"
#include "ui_sprite_cfg_loader_i.h"

ui_sprite_fsm_action_t ui_sprite_cfg_load_action_ctrl_circle(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_cfg_loader_t loader = ctx;
    ui_sprite_ctrl_circle_t ctrl = ui_sprite_ctrl_circle_create(fsm_state, name);

    if (ctrl == NULL) {
        CPE_ERROR(loader->m_em, "%s: create ctrl_circle action: create fail!", ui_sprite_cfg_loader_name(loader));
        return NULL;
    }

    ui_sprite_ctrl_circle_set_keep_send_span(ctrl, cfg_get_float(cfg, "keep-send-span", 0.0f));

    if (ui_sprite_ctrl_circle_set_on_begin(ctrl, cfg_get_string(cfg, "on-begin", NULL)) != 0) {
        CPE_ERROR(
            loader->m_em, "%s: create ctrl_circle action: set on-begin fail",
            ui_sprite_cfg_loader_name(loader));
        ui_sprite_ctrl_circle_free(ctrl);
        return NULL;
    }

    if (ui_sprite_ctrl_circle_set_on_move(ctrl, cfg_get_string(cfg, "on-move", NULL)) != 0) {
        CPE_ERROR(
            loader->m_em, "%s: create ctrl_circle action: set on-move fail",
            ui_sprite_cfg_loader_name(loader));
        ui_sprite_ctrl_circle_free(ctrl);
        return NULL;
    }

    if (ui_sprite_ctrl_circle_set_on_done(ctrl, cfg_get_string(cfg, "on-done", NULL)) != 0) {
        CPE_ERROR(
            loader->m_em, "%s: create ctrl_circle action: set on-done fail",
            ui_sprite_cfg_loader_name(loader));
        ui_sprite_ctrl_circle_free(ctrl);
        return NULL;
    }

    if (ui_sprite_ctrl_circle_set_on_cancel(ctrl, cfg_get_string(cfg, "on-cancel", NULL)) != 0) {
        CPE_ERROR(
            loader->m_em, "%s: create ctrl_circle action: set on-cancel fail",
            ui_sprite_cfg_loader_name(loader));
        ui_sprite_ctrl_circle_free(ctrl);
        return NULL;
    }

    ui_sprite_ctrl_circle_set_negative(ctrl, cfg_get_float(cfg, "negative", ui_sprite_ctrl_circle_negative(ctrl)));
    ui_sprite_ctrl_circle_set_screen_min(ctrl, cfg_get_float(cfg, "screen-min", ui_sprite_ctrl_circle_screen_min(ctrl)));
    ui_sprite_ctrl_circle_set_screen_max(ctrl, cfg_get_float(cfg, "screen-max", ui_sprite_ctrl_circle_screen_max(ctrl)));
    ui_sprite_ctrl_circle_set_max_percent(ctrl, cfg_get_float(cfg, "max-percent", ui_sprite_ctrl_circle_max_percent(ctrl)));
    ui_sprite_ctrl_circle_set_logic_scale(ctrl, cfg_get_float(cfg, "logic-scale", ui_sprite_ctrl_circle_logic_scale(ctrl)));
    ui_sprite_ctrl_circle_set_do_rotate(ctrl, cfg_get_uint8(cfg, "do-rotate", ui_sprite_ctrl_circle_do_rotate(ctrl)));
    ui_sprite_ctrl_circle_set_do_scale(ctrl, cfg_get_uint8(cfg, "do-scale", ui_sprite_ctrl_circle_do_rotate(ctrl)));

    return ui_sprite_fsm_action_from_data(ctrl);
}
