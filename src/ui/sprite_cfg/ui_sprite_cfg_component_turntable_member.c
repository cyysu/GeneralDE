#include "cpe/utils/stream_mem.h"
#include "cpe/cfg/cfg_read.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite_ctrl/ui_sprite_ctrl_turntable_member.h"
#include "ui_sprite_cfg_loader_i.h"

int ui_sprite_cfg_load_component_turntable_member(void * ctx, ui_sprite_component_t component, cfg_t cfg) {
    ui_sprite_cfg_loader_t loader = ctx;
    ui_sprite_ctrl_turntable_member_t turntable_member = ui_sprite_component_data(component);

    if (ui_sprite_ctrl_turntable_member_set_on_select(turntable_member, cfg_get_string(cfg, "on-select", NULL)) != 0) {
        CPE_ERROR(
            loader->m_em, "%s: create turntable_member action: set on-enter fail",
            ui_sprite_cfg_loader_name(loader));
        ui_sprite_ctrl_turntable_member_free(turntable_member);
        return -1;
    }

    if (ui_sprite_ctrl_turntable_member_set_on_unselect(turntable_member, cfg_get_string(cfg, "on-unselect", NULL)) != 0) {
        CPE_ERROR(
            loader->m_em, "%s: create turntable_member action: set on-unselect fail",
            ui_sprite_cfg_loader_name(loader));
        ui_sprite_ctrl_turntable_member_free(turntable_member);
        return -1;
    }

    return 0;
}
