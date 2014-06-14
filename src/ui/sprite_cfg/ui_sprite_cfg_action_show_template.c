#include "cpe/cfg/cfg_read.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_anim/ui_sprite_anim_show_template.h"
#include "ui_sprite_cfg_loader_i.h"

ui_sprite_fsm_action_t ui_sprite_cfg_load_action_show_template(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_cfg_loader_t loader = ctx;
    ui_sprite_anim_show_template_t show_template = ui_sprite_anim_show_template_create(fsm_state, name);
    const char * template_res;

    if (show_template == NULL) {
        CPE_ERROR(loader->m_em, "%s: create show_template action: create fail!", ui_sprite_cfg_loader_name(loader));
        return NULL;
    }

    if ((template_res = cfg_get_string(cfg, "name", NULL)) == NULL) {
        CPE_ERROR(
            loader->m_em, "%s: create show_template action: name not configured!",
            ui_sprite_cfg_loader_name(loader));
        ui_sprite_anim_show_template_free(show_template);
        return NULL;
    }
    ui_sprite_anim_show_template_set_template(show_template, template_res);

    return ui_sprite_fsm_action_from_data(show_template);
}

