#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_entity_attr.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite/ui_sprite_component_meta.h"
#include "ui_sprite_2d_transform_i.h"

ui_sprite_2d_transform_t ui_sprite_2d_transform_create(ui_sprite_entity_t entity) {
    ui_sprite_component_t component = ui_sprite_component_create(entity, UI_SPRITE_2D_TRANSFORM_NAME);
    return component ? ui_sprite_component_data(component) : NULL;
};

ui_sprite_2d_transform_t ui_sprite_2d_transform_find(ui_sprite_entity_t entity) {
    ui_sprite_component_t component = ui_sprite_component_find(entity, UI_SPRITE_2D_TRANSFORM_NAME);
    return component ? ui_sprite_component_data(component) : NULL;
};

void ui_sprite_2d_transform_free(ui_sprite_entity_t entity) {
    ui_sprite_component_t component = ui_sprite_component_find(entity, UI_SPRITE_2D_TRANSFORM_NAME);
    if (component) {
        ui_sprite_component_free(component);
    }
}

static int ui_sprite_2d_transform_init(ui_sprite_component_t component, void * ctx) {
    ui_sprite_2d_transform_t transform = ui_sprite_component_data(component);

    bzero(transform, sizeof(*transform));

    transform->m_module = ctx;
    transform->m_data.transform.scale.x = 1.0f;
    transform->m_data.transform.scale.y = 1.0f;

    return 0;
}

static void ui_sprite_2d_transform_fini(ui_sprite_component_t component, void * ctx) {
    //ui_sprite_2d_transform_t transform = ui_sprite_component_data(component);
}

int ui_sprite_2d_transform_regist(ui_sprite_2d_module_t module) {
    ui_sprite_component_meta_t meta;

    meta = ui_sprite_component_meta_create(
        module->m_repo,
        UI_SPRITE_2D_TRANSFORM_NAME, sizeof(struct ui_sprite_2d_transform));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s register: meta create fail",
            ui_sprite_2d_module_name(module), UI_SPRITE_2D_TRANSFORM_NAME);
        return -1;
    }

    ui_sprite_component_meta_set_data_meta(
        meta,
        module->m_meta_transform_data,
        CPE_ENTRY_START(ui_sprite_2d_transform, m_data),
        CPE_ENTRY_SIZE(ui_sprite_2d_transform, m_data));

    ui_sprite_component_meta_set_init_fun(meta, ui_sprite_2d_transform_init, module);
    ui_sprite_component_meta_set_free_fun(meta, ui_sprite_2d_transform_fini, module);

    return 0;
}

void ui_sprite_2d_transform_unregist(ui_sprite_2d_module_t module) {
    ui_sprite_component_meta_t meta;

    meta = ui_sprite_component_meta_find(module->m_repo, UI_SPRITE_2D_TRANSFORM_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s unregister: meta not exist",
            ui_sprite_2d_module_name(module), UI_SPRITE_2D_TRANSFORM_NAME);
        return;
    }

    ui_sprite_component_meta_free(meta);
}

UI_SPRITE_2D_PAIR ui_sprite_2d_transform_pos(ui_sprite_2d_transform_t transform, uint8_t pos_policy) {
    UI_SPRITE_2D_PAIR r = transform->m_data.transform.pos;

    switch(pos_policy) {
    case UI_SPRITE_2D_TRANSFORM_POS_ORIGIN:
        break;
    case UI_SPRITE_2D_TRANSFORM_POS_TOP_LEFT:
        r.x += transform->m_data.transform.rect_lt.x * transform->m_data.transform.scale.x;
        r.y += transform->m_data.transform.rect_lt.y * transform->m_data.transform.scale.y;
        break;
    case UI_SPRITE_2D_TRANSFORM_POS_TOP_CENTER:
        r.x += (transform->m_data.transform.rect_lt.x + transform->m_data.transform.rect_rb.x) / 2.0f * transform->m_data.transform.scale.x;
        r.y += transform->m_data.transform.rect_lt.y * transform->m_data.transform.scale.y;
        break;
    case UI_SPRITE_2D_TRANSFORM_POS_TOP_RIGHT:
        r.x += transform->m_data.transform.rect_rb.x * transform->m_data.transform.scale.x;
        r.y += transform->m_data.transform.rect_lt.y * transform->m_data.transform.scale.y;
        break;
    case UI_SPRITE_2D_TRANSFORM_POS_CENTER_LEFT:
        r.x += transform->m_data.transform.rect_lt.x * transform->m_data.transform.scale.x;
        r.y += (transform->m_data.transform.rect_lt.y + transform->m_data.transform.rect_rb.y) / 2.0f * transform->m_data.transform.scale.y;
        break;
    case UI_SPRITE_2D_TRANSFORM_POS_CENTER:
        r.x += (transform->m_data.transform.rect_lt.x + transform->m_data.transform.rect_rb.x) / 2.0f * transform->m_data.transform.scale.x;
        r.y += (transform->m_data.transform.rect_lt.y + transform->m_data.transform.rect_rb.y) / 2.0f * transform->m_data.transform.scale.y;
        break;
    case UI_SPRITE_2D_TRANSFORM_POS_CENTER_RIGHT:
        r.x += transform->m_data.transform.rect_rb.x * transform->m_data.transform.scale.x;
        r.y += (transform->m_data.transform.rect_lt.y + transform->m_data.transform.rect_rb.y) / 2.0f * transform->m_data.transform.scale.y;
        break;
    case UI_SPRITE_2D_TRANSFORM_POS_BOTTOM_LEFT:
        r.x += transform->m_data.transform.rect_lt.x * transform->m_data.transform.scale.x;
        r.y += transform->m_data.transform.rect_rb.y * transform->m_data.transform.scale.y;
        break;
    case UI_SPRITE_2D_TRANSFORM_POS_BOTTOM_CENTER:
        r.x += (transform->m_data.transform.rect_lt.x + transform->m_data.transform.rect_rb.x) / 2.0f * transform->m_data.transform.scale.x;
        r.y += transform->m_data.transform.rect_rb.y * transform->m_data.transform.scale.y;
        break;
    case UI_SPRITE_2D_TRANSFORM_POS_BOTTOM_RIGHT:
        r.x += transform->m_data.transform.rect_rb.x * transform->m_data.transform.scale.x;
        r.y += transform->m_data.transform.rect_rb.y * transform->m_data.transform.scale.y;
        break;
    default: {
        ui_sprite_component_t component = ui_sprite_component_from_data(transform);
        ui_sprite_entity_t entity = ui_sprite_component_entity(component);

        CPE_ERROR(
            transform->m_module->m_em, "entity %d(%s): transform: get pos policy %d is unknown",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), pos_policy);

        r.x = 0.0f;
        r.y = 0.0f;
        break;
    }
    }

    return r;
}

void ui_sprite_2d_transform_set_pos(ui_sprite_2d_transform_t transform, UI_SPRITE_2D_PAIR pos) {
    if (pos.x != transform->m_data.transform.pos.x || pos.y != transform->m_data.transform.pos.y) {
        ui_sprite_component_t component = ui_sprite_component_from_data(transform);
        ui_sprite_entity_t entity = ui_sprite_component_entity(component);

        transform->m_data.transform.pos = pos;

        if (ui_sprite_component_is_active(component)) {
            ui_sprite_entity_notify_attr_updated(entity, "transform.pos");
        }
    }
}

UI_SPRITE_2D_PAIR ui_sprite_2d_transform_scale(ui_sprite_2d_transform_t transform) {
    return transform->m_data.transform.scale;
}

void ui_sprite_2d_transform_set_scale(ui_sprite_2d_transform_t transform, UI_SPRITE_2D_PAIR scale) {
    if (scale.x != transform->m_data.transform.scale.x || scale.y != transform->m_data.transform.scale.y) {
        ui_sprite_component_t component = ui_sprite_component_from_data(transform);
        ui_sprite_entity_t entity = ui_sprite_component_entity(component);

        transform->m_data.transform.scale = scale;

        if (ui_sprite_component_is_active(component)) {
            ui_sprite_entity_notify_attr_updated(entity, "transform.scale");
        }
    }
}

float ui_sprite_2d_transform_angle(ui_sprite_2d_transform_t transform) {
    return transform->m_data.transform.angle;
}

void ui_sprite_2d_transform_set_angle(ui_sprite_2d_transform_t transform, float angle) {
    if (angle != transform->m_data.transform.angle) {
        ui_sprite_component_t component = ui_sprite_component_from_data(transform);
        ui_sprite_entity_t entity = ui_sprite_component_entity(component);

        transform->m_data.transform.angle = angle;

        if (ui_sprite_component_is_active(component)) {
            ui_sprite_entity_notify_attr_updated(entity, "transform.angle");
        }
    }
}

void ui_sprite_2d_transform_merge_rect(ui_sprite_2d_transform_t transform, UI_SPRITE_2D_PAIR lt, UI_SPRITE_2D_PAIR rb) {
    uint8_t updated_lt = 0;
    uint8_t updated_rb = 0;

    if (lt.x < transform->m_data.transform.rect_lt.x) {
        transform->m_data.transform.rect_lt.x = lt.x;
        updated_lt = 1;
    }

    if (lt.y < transform->m_data.transform.rect_lt.y) {
        transform->m_data.transform.rect_lt.y = lt.y;
        updated_lt = 1;
    }

    if (rb.x > transform->m_data.transform.rect_rb.x) {
        transform->m_data.transform.rect_rb.x = rb.x;
        updated_rb = 1;
    }

    if (rb.y > transform->m_data.transform.rect_rb.y) {
        transform->m_data.transform.rect_rb.y = rb.y;
        updated_rb = 1;
    }

    if (updated_lt || updated_rb) {
        ui_sprite_component_t component = ui_sprite_component_from_data(transform);
        ui_sprite_entity_t entity = ui_sprite_component_entity(component);

        if (ui_sprite_component_is_active(component)) {
            if (updated_lt) {
                ui_sprite_entity_notify_attr_updated(entity, "transform.rect_lt");
            }

            if (updated_rb) {
                ui_sprite_entity_notify_attr_updated(entity, "transform.rect_rb");
            }
        }
    }
}

UI_SPRITE_2D_PAIR ui_sprite_2d_transform_rect_lt(ui_sprite_2d_transform_t transform) {
    return transform->m_data.transform.rect_lt;
}

UI_SPRITE_2D_PAIR ui_sprite_2d_transform_rect_rb(ui_sprite_2d_transform_t transform) {
    return transform->m_data.transform.rect_rb;
}

uint8_t ui_sprite_2d_transform_flip_x(ui_sprite_2d_transform_t transform) {
    return transform->m_data.transform.flip_x;
}

void ui_sprite_2d_transform_set_flip_x(ui_sprite_2d_transform_t transform, uint8_t flip_x) {
    if (transform->m_data.transform.flip_x != flip_x) {
        ui_sprite_component_t component = ui_sprite_component_from_data(transform);
        ui_sprite_entity_t entity = ui_sprite_component_entity(component);

        transform->m_data.transform.flip_x = flip_x;

        if (ui_sprite_component_is_active(component)) {
            ui_sprite_entity_notify_attr_updated(entity, "transform.flip_x");
        }
    }
}

uint8_t ui_sprite_2d_transform_flip_y(ui_sprite_2d_transform_t transform) {
    return transform->m_data.transform.flip_y;
}

void ui_sprite_2d_transform_set_flip_y(ui_sprite_2d_transform_t transform, uint8_t flip_y) {
    if (transform->m_data.transform.flip_y != flip_y) {
        ui_sprite_component_t component = ui_sprite_component_from_data(transform);
        ui_sprite_entity_t entity = ui_sprite_component_entity(component);

        transform->m_data.transform.flip_y = flip_y;

        if (ui_sprite_component_is_active(component)) {
            ui_sprite_entity_notify_attr_updated(entity, "transform.flip_y");
        }
    }
}

static const char * s_pos_policy_defs[] = {
    "unknown-pos-policy",
    "origin",
    "top-left",
    "top-center",
    "top-right",
    "center-left",
    "center",
    "center-right",
    "bottom-left",
    "bottom-center",
    "bottom-right",
};

uint8_t ui_sprite_2d_transform_pos_policy_from_str(const char * str_pos_policy) {
    uint8_t i;

    if (str_pos_policy[0] == 0) return UI_SPRITE_2D_TRANSFORM_POS_ORIGIN;

    for(i = 1; i < CPE_ARRAY_SIZE(s_pos_policy_defs); ++i) {
        if (strcmp(s_pos_policy_defs[i], str_pos_policy) == 0) return i;
    }

    return 0;
}

const char * ui_sprite_2d_transform_pos_policy_to_str(uint8_t pos_policy) {
    return s_pos_policy_defs[pos_policy >= CPE_ARRAY_SIZE(s_pos_policy_defs) ? 0 : pos_policy];
}

const char * UI_SPRITE_2D_TRANSFORM_NAME = "Transform";
