#include "cpe/utils/stream_mem.h"
#include "cpe/cfg/cfg_read.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui/sprite_anim/ui_sprite_anim_sch.h"
#include "ui/sprite_anim/ui_sprite_anim_def.h"
#include "ui/sprite_anim/ui_sprite_anim_group.h"
#include "ui/sprite_anim/ui_sprite_anim_template.h"
#include "ui/sprite_anim/ui_sprite_anim_template_binding.h"
#include "ui_sprite_cfg_loader_i.h"

static int ui_sprite_cfg_load_component_animation_groups(ui_sprite_cfg_loader_t loader, ui_sprite_anim_sch_t animation, cfg_t cfg);
static int ui_sprite_cfg_load_component_animation_resources(ui_sprite_cfg_loader_t loader, ui_sprite_anim_sch_t animation, cfg_t cfg);
static int ui_sprite_cfg_load_component_animation_templates(ui_sprite_cfg_loader_t loader, ui_sprite_anim_sch_t animation, cfg_t cfg);

int ui_sprite_cfg_load_component_animation(void * ctx, ui_sprite_component_t component, cfg_t cfg) {
    ui_sprite_cfg_loader_t loader = ctx;
    ui_sprite_anim_sch_t animation = ui_sprite_component_data(component);
    const char * layer;

    if ((layer = cfg_get_string(cfg, "layer", NULL))) {
        ui_sprite_anim_sch_set_default_layer(animation, layer);
    }

    if (ui_sprite_cfg_load_component_animation_groups(loader, animation, cfg_find_cfg(cfg, "groups")) != 0) {
        return -1;
    }

    if (ui_sprite_cfg_load_component_animation_resources(loader, animation, cfg_find_cfg(cfg, "resources")) != 0) {
        return -1;
    }

    if (ui_sprite_cfg_load_component_animation_templates(loader, animation, cfg_find_cfg(cfg, "templates")) != 0) {
        return -1;
    }

    return 0;
}

static int ui_sprite_cfg_load_component_animation_groups(ui_sprite_cfg_loader_t loader, ui_sprite_anim_sch_t animation, cfg_t cfg) {
    struct cfg_it group_cfg_it;
    cfg_t group_cfg;
    int32_t index = 0;
    ui_sprite_anim_group_t group;

    cfg_it_init(&group_cfg_it, cfg);
    while((group_cfg = cfg_it_next(&group_cfg_it))) {
        const char * group_name;
        const char * base_pos_str;
        cfg_t pos_cfg;

        if ((group_name = cfg_get_string(group_cfg, "name", NULL)) == NULL) {
            CPE_ERROR(
                loader->m_em, "%s: create animation component: group (%d) name not configured!",
                ui_sprite_cfg_loader_name(loader), index);
            return -1;
        }

        group = ui_sprite_anim_group_create(animation, group_name);
        if (group == NULL) {
            CPE_ERROR(
                loader->m_em, "%s: create animation component: group %s: create fail!",
                ui_sprite_cfg_loader_name(loader), group_name);
            return -1;
        }

        if ((pos_cfg = cfg_find_cfg(group_cfg, "pos-adj"))) {
            UI_SPRITE_2D_PAIR old_pos = ui_sprite_anim_group_pos_adj(group);
            UI_SPRITE_2D_PAIR pos_adj;

            pos_adj.x = cfg_get_float(pos_cfg, "x", old_pos.x);
            pos_adj.y = cfg_get_float(pos_cfg, "y", old_pos.y);

            ui_sprite_anim_group_set_pos_adj(group, pos_adj);
        }

        ui_sprite_anim_group_set_accept_scale(
            group,
            cfg_get_uint8(group_cfg, "accept-scale", ui_sprite_anim_group_accept_scale(group)));

        ui_sprite_anim_group_set_adj_accept_scale(
            group,
            cfg_get_uint8(group_cfg, "pos-adj-accept-scale", ui_sprite_anim_group_adj_accept_scale(group)));

        if ((base_pos_str = cfg_get_string(group_cfg, "base-pos", NULL))) {
            uint8_t base_pos = ui_sprite_2d_transform_pos_policy_from_str(base_pos_str);
            if (base_pos == 0) {
                CPE_ERROR(
                    loader->m_em, "%s: create animation component: group %s: base-pos %s error!",
                    ui_sprite_cfg_loader_name(loader), group_name, base_pos_str);
                return -1;
            }

            ui_sprite_anim_group_set_base_pos(group, base_pos);
        }

        ++index;
    }

    return 0;
}

static int ui_sprite_cfg_load_component_animation_resources(
    ui_sprite_cfg_loader_t loader, ui_sprite_anim_sch_t animation, cfg_t cfg)
{
    struct cfg_it animation_cfg_it;
    cfg_t animation_cfg;
    int32_t index = 0;

    cfg_it_init(&animation_cfg_it, cfg);
    while((animation_cfg = cfg_it_next(&animation_cfg_it))) {
        const char * animation_res;
        const char * animation_name;

        if ((animation_name = cfg_get_string(animation_cfg, "name", NULL)) == NULL) {
            CPE_ERROR(
                loader->m_em, "%s: create animation component: (%d) name not configured!",
                ui_sprite_cfg_loader_name(loader), index);
            return -1;
        }

        if ((animation_res = cfg_get_string(animation_cfg, "res", NULL)) == NULL) {
            CPE_ERROR(
                loader->m_em, "%s: create animation component: (%d) res not configured!",
                ui_sprite_cfg_loader_name(loader), index);
            return -1;
        }

        if (ui_sprite_anim_def_create(
                animation, animation_name, animation_res, cfg_get_uint8(animation_cfg, "auto-start", 0))
            == NULL)
        {
            CPE_ERROR(
                loader->m_em, "%s: create animation component: (%d) add animation %s ==> %s fail!",
                ui_sprite_cfg_loader_name(loader), index,
                animation_name, animation_res);
            return -1;
        }

        ++index;
    }

    return 0;
}

static int ui_sprite_cfg_load_component_animation_templates(ui_sprite_cfg_loader_t loader, ui_sprite_anim_sch_t animation, cfg_t cfg) {
    struct cfg_it template_cfg_it;
    cfg_t template_cfg;
    int32_t index = 0;

    cfg_it_init(&template_cfg_it, cfg);
    while((template_cfg = cfg_it_next(&template_cfg_it))) {
        const char * template_res;
        const char * template_name;
        ui_sprite_anim_template_t tpl;
        struct cfg_it binding_cfg_it;
        cfg_t binding_cfg;
        
        if ((template_name = cfg_get_string(template_cfg, "name", NULL)) == NULL) {
            CPE_ERROR(
                loader->m_em, "%s: create animation component: template (%d) name not configured!",
                ui_sprite_cfg_loader_name(loader), index);
            return -1;
        }

        if ((template_res = cfg_get_string(template_cfg, "res", NULL)) == NULL) {
            CPE_ERROR(
                loader->m_em, "%s: create animation component: template %s: res not configured!",
                ui_sprite_cfg_loader_name(loader), template_name);
            return -1;
        }

        tpl = ui_sprite_anim_template_create(
            animation,
            template_name, cfg_get_string(template_cfg, "group", ""),
            template_res);
        if (tpl == NULL) {
            CPE_ERROR(
                loader->m_em, "%s: create animation component: template %s: create fail!",
                ui_sprite_cfg_loader_name(loader), template_name);
            return -1;
        }

        cfg_it_init(&binding_cfg_it, cfg_find_cfg(template_cfg, "bindings"));
        while((binding_cfg = cfg_it_next(&binding_cfg_it))) {
            struct cfg_it attr_cfg_it;
            cfg_t attr_cfg;
            const char * control_name;

            control_name = cfg_get_string(binding_cfg, "control", NULL);
            if (control_name == NULL) {
                CPE_ERROR(
                    loader->m_em, "%s: create animation component: template %s: binding control name not set!",
                    ui_sprite_cfg_loader_name(loader), template_name);
                return -1;
            }

            cfg_it_init(&attr_cfg_it, binding_cfg);
            while((attr_cfg = cfg_it_next(&attr_cfg_it))) {
                const char * attr_name = cfg_name(attr_cfg);
                const char * value;
                char value_buf[64];

                if (strcmp(attr_name, "control") == 0) continue;

                value = cfg_as_string(attr_cfg, NULL);
                if (value == NULL) {
                    struct write_stream_mem s = CPE_WRITE_STREAM_MEM_INITIALIZER(value_buf, sizeof(value_buf));
                    cfg_print_inline(attr_cfg, (write_stream_t)&s);
                    stream_putc((write_stream_t)&s, 0);
                    value = value_buf;
                }

                if (ui_sprite_anim_template_binding_create(tpl, control_name, attr_name, value) == NULL) {
                    CPE_ERROR(
                        loader->m_em, "%s: create animation component: template %s: binding control %s (%s=%s) fail!",
                        ui_sprite_cfg_loader_name(loader), template_name, control_name, attr_name, value);
                    return -1;
                }
            }
        }

        ++index;
    }

    return 0;
}
