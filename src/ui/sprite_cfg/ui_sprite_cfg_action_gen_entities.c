#include "cpe/cfg/cfg_read.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_basic/ui_sprite_basic_gen_entities.h"
#include "ui_sprite_cfg_loader_i.h"

ui_sprite_fsm_action_t ui_sprite_cfg_load_action_gen_entities(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_cfg_loader_t loader = ctx;
    ui_sprite_basic_gen_entities_t gen_entities = ui_sprite_basic_gen_entities_create(fsm_state, name);
    const char * proto;
    const char * attrs;

    if (gen_entities == NULL) {
        CPE_ERROR(loader->m_em, "%s: create gen_entities action: create fail!", ui_sprite_cfg_loader_name(loader));
        return NULL;
    }

    if ((proto = cfg_get_string(cfg, "proto", NULL)) == NULL) {
        CPE_ERROR(
            loader->m_em, "%s: create gen_entities action: set proto fail",
            ui_sprite_cfg_loader_name(loader));
        ui_sprite_basic_gen_entities_free(gen_entities);
        return NULL;
    }

    ui_sprite_basic_gen_entities_set_proto(gen_entities, proto);
    ui_sprite_basic_gen_entities_set_wait_stop(gen_entities, cfg_get_uint8(cfg, "wait-stop", 0));
    ui_sprite_basic_gen_entities_set_do_destory(gen_entities, cfg_get_uint8(cfg, "do-destory", 0));

    if ((attrs = cfg_get_string(cfg, "attributes", NULL))) {
        if (ui_sprite_basic_gen_entities_set_attrs(gen_entities, attrs) != 0) {
            CPE_ERROR(
                loader->m_em, "%s: create gen_entities action: set attributes %s fail",
                ui_sprite_cfg_loader_name(loader), attrs);
            ui_sprite_basic_gen_entities_free(gen_entities);
        }
    }

    return ui_sprite_fsm_action_from_data(gen_entities);
}

