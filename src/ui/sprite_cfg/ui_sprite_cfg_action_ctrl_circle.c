#include "cpe/cfg/cfg_read.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_ctrl/ui_sprite_ctrl_circle.h"
#include "ui_sprite_cfg_loader_i.h"

ui_sprite_fsm_action_t ui_sprite_cfg_load_action_ctrl_circle(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_cfg_loader_t loader = ctx;
    ui_sprite_ctrl_circle_t ctrl = ui_sprite_ctrl_circle_create(fsm_state, name);
    cfg_t child_cfg;

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

    if ((child_cfg = cfg_find_cfg(cfg, "screen-range"))) {
        if (ui_sprite_ctrl_circle_set_screen_range(
                ctrl, cfg_get_float(child_cfg, "min", 0.0f), cfg_get_float(child_cfg, "max", 0.0f))
            != 0)
        {
            CPE_ERROR(
                loader->m_em, "%s: create ctrl_circle action: set screen-range fail",
                ui_sprite_cfg_loader_name(loader));
            ui_sprite_ctrl_circle_free(ctrl);
            return NULL;
        }
    }
    else {
        CPE_ERROR(
            loader->m_em, "%s: create ctrl_circle action: screen-range not configured",
            ui_sprite_cfg_loader_name(loader));
        ui_sprite_ctrl_circle_free(ctrl);
        return NULL;
    }

    if ((child_cfg = cfg_find_cfg(cfg, "angle-range"))) {
        if (ui_sprite_ctrl_circle_set_angle_range(
                ctrl, cfg_get_float(child_cfg, "min", -180.0f), cfg_get_float(child_cfg, "max", 180.0f))
            != 0)
        {
            CPE_ERROR(
                loader->m_em, "%s: create ctrl_circle action: set angle-range fail",
                ui_sprite_cfg_loader_name(loader));
            ui_sprite_ctrl_circle_free(ctrl);
            return NULL;
        }
    }
    else {
        CPE_ERROR(
            loader->m_em, "%s: create ctrl_circle action: angle-range not configured",
            ui_sprite_cfg_loader_name(loader));
        ui_sprite_ctrl_circle_free(ctrl);
        return NULL;
    }

    if ((child_cfg = cfg_find_cfg(cfg, "logic-range"))) {
        if (ui_sprite_ctrl_circle_set_logic_range(
                ctrl, cfg_get_float(child_cfg, "min", 0.0f), cfg_get_float(child_cfg, "max", 0.0f))
            != 0)
        {
            CPE_ERROR(
                loader->m_em, "%s: create ctrl_circle action: set logic-range fail",
                ui_sprite_cfg_loader_name(loader));
            ui_sprite_ctrl_circle_free(ctrl);
            return NULL;
        }
    }
    else {
        CPE_ERROR(
            loader->m_em, "%s: create ctrl_circle action: logic-range not configured",
            ui_sprite_cfg_loader_name(loader));
        ui_sprite_ctrl_circle_free(ctrl);
        return NULL;
    }

    ui_sprite_ctrl_circle_set_negative(ctrl, cfg_get_uint8(cfg, "negative", ui_sprite_ctrl_circle_negative(ctrl)));
    ui_sprite_ctrl_circle_set_do_rotate(ctrl, cfg_get_uint8(cfg, "do-rotate", ui_sprite_ctrl_circle_do_rotate(ctrl)));
    ui_sprite_ctrl_circle_set_do_scale(ctrl, cfg_get_uint8(cfg, "do-scale", ui_sprite_ctrl_circle_do_scale(ctrl)));
    ui_sprite_ctrl_circle_set_logic_base(ctrl, cfg_get_float(cfg, "logic-base", ui_sprite_ctrl_circle_logic_base(ctrl)));
    ui_sprite_ctrl_circle_set_cancel_distance(ctrl, cfg_get_float(cfg, "cancel-distance", ui_sprite_ctrl_circle_cancel_distance(ctrl)));

    return ui_sprite_fsm_action_from_data(ctrl);
}
