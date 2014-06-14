#include "cpe/cfg/cfg_read.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_basic/ui_sprite_basic_set_attrs.h"
#include "ui_sprite_cfg_loader_i.h"

ui_sprite_fsm_action_t ui_sprite_cfg_load_action_set_attrs(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_cfg_loader_t loader = ctx;
    ui_sprite_basic_set_attrs_t set_attrs;
    const char * setter;

    setter = cfg_get_string(cfg, "setter", NULL);
    if (setter == NULL) {
        CPE_ERROR(
            loader->m_em, "%s: create set_attrs action: setter not configured",
            ui_sprite_cfg_loader_name(loader));
        return NULL;
    }

    set_attrs = ui_sprite_basic_set_attrs_create(fsm_state, name);
    if (set_attrs == NULL) {
        CPE_ERROR(loader->m_em, "%s: create set_attrs action: create fail!", ui_sprite_cfg_loader_name(loader));
        return NULL;
    }

    if (ui_sprite_basic_set_attrs_set_setter(set_attrs, setter) != 0) {
        CPE_ERROR(
            loader->m_em, "%s: create set_attrs action: set setter %s fail",
            ui_sprite_cfg_loader_name(loader), setter);
        ui_sprite_basic_set_attrs_free(set_attrs);
        return NULL;
    }

    return ui_sprite_fsm_action_from_data(set_attrs);
}

