#include "cpe/cfg/cfg_read.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_touch/ui_sprite_touch_scale.h"
#include "ui_sprite_cfg_loader_i.h"

ui_sprite_fsm_action_t ui_sprite_cfg_load_action_touch_scale(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_cfg_loader_t loader = ctx;
    ui_sprite_touch_scale_t touch_scale = ui_sprite_touch_scale_create(fsm_state, name);
    int32_t finger_count;

    if (touch_scale == NULL) {
        CPE_ERROR(loader->m_em, "%s: create touch_scale action: create fail!", ui_sprite_cfg_loader_name(loader));
        return NULL;
    }

    finger_count = cfg_get_int32(cfg, "finger-count", 1);
    if (finger_count <= 0 || finger_count > UI_SPRITE_TOUCH_MAX_FINGER_COUNT) {
        CPE_ERROR(
            loader->m_em, "%s: create touch_scale action: finger-count %d error!",
            ui_sprite_cfg_loader_name(loader), finger_count);
        ui_sprite_touch_scale_free(touch_scale);
        return NULL;
    }

    if (ui_sprite_touch_scale_set_finger_count(touch_scale, finger_count) != 0) {
        CPE_ERROR(
            loader->m_em, "%s: create touch_scale action: set finger-count %d error!",
            ui_sprite_cfg_loader_name(loader), finger_count);
        ui_sprite_touch_scale_free(touch_scale);
        return NULL;
    }

    if (ui_sprite_touch_scale_set_is_capture(
            touch_scale, cfg_get_uint8(cfg, "is-capture", ui_sprite_touch_scale_is_capture(touch_scale)))
            != 0)
    {
        CPE_ERROR(
            loader->m_em, "%s: create touch_scale action: set is-capture error!",
            ui_sprite_cfg_loader_name(loader));
        ui_sprite_touch_scale_free(touch_scale);
        return NULL;
    }

    if (ui_sprite_touch_scale_set_is_grab(
            touch_scale, cfg_get_uint8(cfg, "is-grab", ui_sprite_touch_scale_is_grab(touch_scale)))
            != 0)
    {
        CPE_ERROR(
            loader->m_em, "%s: create touch_scale action: set is-grab error!",
            ui_sprite_cfg_loader_name(loader));
        ui_sprite_touch_scale_free(touch_scale);
        return NULL;
    }

    ui_sprite_touch_scale_set_threshold(touch_scale, cfg_get_uint8(cfg, "threshold", ui_sprite_touch_scale_threshold(touch_scale)));

    if (ui_sprite_touch_scale_set_on_begin(touch_scale, cfg_get_string(cfg, "on-begin", NULL)) != 0) {
        CPE_ERROR(
            loader->m_em, "%s: create touch_scale action: set on-begin fail",
            ui_sprite_cfg_loader_name(loader));
        ui_sprite_touch_scale_free(touch_scale);
        return NULL;
    }

    if (ui_sprite_touch_scale_set_on_scale(touch_scale, cfg_get_string(cfg, "on-scale", NULL)) != 0) {
        CPE_ERROR(
            loader->m_em, "%s: create touch_scale action: set on-scale fail",
            ui_sprite_cfg_loader_name(loader));
        ui_sprite_touch_scale_free(touch_scale);
        return NULL;
    }

    if (ui_sprite_touch_scale_set_on_end(touch_scale, cfg_get_string(cfg, "on-end", NULL)) != 0) {
        CPE_ERROR(
            loader->m_em, "%s: create touch_scale action: set on-end fail",
            ui_sprite_cfg_loader_name(loader));
        ui_sprite_touch_scale_free(touch_scale);
        return NULL;
    }

    if (ui_sprite_touch_scale_set_on_cancel(touch_scale, cfg_get_string(cfg, "on-cancel", NULL)) != 0) {
        CPE_ERROR(
            loader->m_em, "%s: create touch_scale action: set on-cancel fail",
            ui_sprite_cfg_loader_name(loader));
        ui_sprite_touch_scale_free(touch_scale);
        return NULL;
    }

    return ui_sprite_fsm_action_from_data(touch_scale);
}

