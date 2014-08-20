#include "cpe/pal/pal_strings.h"
#include "cpe/cfg/cfg.h"
#include "ui_sprite_cfg_value_generator_load_i.h"

int ui_sprite_cfg_value_generator_load(
    ui_sprite_cfg_loader_t loader,
    UI_SPRITE_BASIC_VALUE_GENEARTOR_DEF * def,
    cfg_t cfg)
{
    cfg_t supplies_cfg;
    const char * type;

    bzero(def, sizeof(*def));

    type = cfg_get_string(cfg, "type", NULL);
    if (type == NULL) {
        CPE_ERROR(loader->m_em, "%s: value generator load: type not configured!", ui_sprite_cfg_loader_name(loader));
        return -1;
    }

    if (strcmp(type, "in-range") == 0) {
        const char * min = cfg_get_string(cfg, "range.min", NULL);
        const char * max = cfg_get_string(cfg, "range.max", NULL);

        if (min == NULL) {
            CPE_ERROR(loader->m_em, "%s: value generator load: type %s range.min not configured!", ui_sprite_cfg_loader_name(loader), type);
            return -1;
        }

        if (max == NULL) {
            CPE_ERROR(loader->m_em, "%s: value generator load: type %s range.min not configured!", ui_sprite_cfg_loader_name(loader), type);
            return -1;
        }

        def->type = UI_SPRITE_BASIC_VALUE_GENEARTOR_TYPE_IN_RANGE;
        strncpy(def->data.in_range.min, min, sizeof(def->data.in_range.min));
        strncpy(def->data.in_range.max, max, sizeof(def->data.in_range.max));
    }
    else {
        CPE_ERROR(loader->m_em, "%s: value generator load: type %s is unknown!", ui_sprite_cfg_loader_name(loader), type);
        return -1;
    }

    strncpy(def->once_gen_count, cfg_get_string(cfg, "once-gen-count", ""), sizeof(def->once_gen_count));

    if ((supplies_cfg = cfg_find_cfg(cfg, "supplies"))) {
        struct cfg_it supply_it;
        cfg_t supply_cfg;

        cfg_it_init(&supply_it, supplies_cfg);

        while((supply_cfg = cfg_it_next(&supply_it))) {
            const char * attr_name = cfg_as_string(supply_cfg, NULL);

            if (attr_name == NULL) {
                CPE_ERROR(
                    loader->m_em, "%s: value generator load: supply format error!",
                    ui_sprite_cfg_loader_name(loader));
                return -1;
            }

            strncpy(def->supplies[def->supply_count], attr_name, sizeof(def->supplies[def->supply_count]));
            def->supply_count++;
        }
    }

    return 0;
}


