#include "cpe/cfg/cfg_read.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_basic/ui_sprite_basic_join_group.h"
#include "ui_sprite_cfg_loader_i.h"

ui_sprite_fsm_action_t ui_sprite_cfg_load_action_join_group(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_cfg_loader_t loader = ctx;
    ui_sprite_basic_join_group_t join_group = ui_sprite_basic_join_group_create(fsm_state, name);
    const char * group_name;

    group_name = cfg_get_string(cfg, "group-name", NULL);
    if (group_name == NULL) {
        CPE_ERROR(
            loader->m_em, "%s: create join_group action: group-name not configured",
            ui_sprite_cfg_loader_name(loader));
        return NULL;
    }

    if (join_group == NULL) {
        CPE_ERROR(loader->m_em, "%s: create join_group action: create fail!", ui_sprite_cfg_loader_name(loader));
        return NULL;
    }

    if (ui_sprite_basic_join_group_set_name(join_group, group_name) != 0) {
        CPE_ERROR(
            loader->m_em, "%s: create join_group action: set group-name %s fail",
            ui_sprite_cfg_loader_name(loader), group_name);
        ui_sprite_basic_join_group_free(join_group);
        return NULL;
    }

    return ui_sprite_fsm_action_from_data(join_group);
}

