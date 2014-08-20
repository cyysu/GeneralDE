#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_entity_attr.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite/ui_sprite_component_meta.h"
#include "ui/sprite_2d/ui_sprite_2d_utils.h"
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

UI_SPRITE_2D_PAIR ui_sprite_2d_transform_local_pos(ui_sprite_2d_transform_t transform, uint8_t pos_policy, uint8_t adj_type) {
    UI_SPRITE_2D_PAIR r;

    switch(pos_policy) {
    case UI_SPRITE_2D_TRANSFORM_POS_F_TOP_LEFT:
        pos_policy =
            transform->m_data.transform.flip_x
            ? (transform->m_data.transform.flip_y
               ? UI_SPRITE_2D_TRANSFORM_POS_BOTTOM_RIGHT
               : UI_SPRITE_2D_TRANSFORM_POS_TOP_RIGHT)
            : (transform->m_data.transform.flip_y
               ? UI_SPRITE_2D_TRANSFORM_POS_BOTTOM_LEFT
               : UI_SPRITE_2D_TRANSFORM_POS_TOP_LEFT);
        break;
    case UI_SPRITE_2D_TRANSFORM_POS_F_TOP_CENTER:
        pos_policy = 
            transform->m_data.transform.flip_y
            ? UI_SPRITE_2D_TRANSFORM_POS_BOTTOM_CENTER
            : UI_SPRITE_2D_TRANSFORM_POS_TOP_CENTER;
        break;
    case UI_SPRITE_2D_TRANSFORM_POS_F_TOP_RIGHT:
        pos_policy =
            transform->m_data.transform.flip_x
            ? (transform->m_data.transform.flip_y
               ? UI_SPRITE_2D_TRANSFORM_POS_BOTTOM_LEFT
               : UI_SPRITE_2D_TRANSFORM_POS_TOP_LEFT)
            : (transform->m_data.transform.flip_y
               ? UI_SPRITE_2D_TRANSFORM_POS_BOTTOM_RIGHT
               : UI_SPRITE_2D_TRANSFORM_POS_TOP_RIGHT);
        break;
    case UI_SPRITE_2D_TRANSFORM_POS_F_CENTER_LEFT:
        pos_policy =
            transform->m_data.transform.flip_x
            ? UI_SPRITE_2D_TRANSFORM_POS_CENTER_RIGHT
            : UI_SPRITE_2D_TRANSFORM_POS_CENTER_LEFT;
        break;
    case UI_SPRITE_2D_TRANSFORM_POS_F_CENTER_RIGHT:
        pos_policy =
            transform->m_data.transform.flip_x
            ? UI_SPRITE_2D_TRANSFORM_POS_CENTER_LEFT
            : UI_SPRITE_2D_TRANSFORM_POS_CENTER_RIGHT;
        break;
    case UI_SPRITE_2D_TRANSFORM_POS_F_BOTTOM_LEFT:
        pos_policy =
            transform->m_data.transform.flip_x
            ? (transform->m_data.transform.flip_y
               ? UI_SPRITE_2D_TRANSFORM_POS_TOP_RIGHT
               : UI_SPRITE_2D_TRANSFORM_POS_BOTTOM_RIGHT)
            : (transform->m_data.transform.flip_y
               ? UI_SPRITE_2D_TRANSFORM_POS_TOP_LEFT
               : UI_SPRITE_2D_TRANSFORM_POS_BOTTOM_LEFT);
        break;
    case UI_SPRITE_2D_TRANSFORM_POS_F_BOTTOM_CENTER:
        pos_policy = 
            transform->m_data.transform.flip_y
            ? UI_SPRITE_2D_TRANSFORM_POS_TOP_CENTER
            : UI_SPRITE_2D_TRANSFORM_POS_BOTTOM_CENTER;
        break;
    case UI_SPRITE_2D_TRANSFORM_POS_F_BOTTOM_RIGHT:
        pos_policy =
            transform->m_data.transform.flip_x
            ? (transform->m_data.transform.flip_y
               ? UI_SPRITE_2D_TRANSFORM_POS_TOP_LEFT
               : UI_SPRITE_2D_TRANSFORM_POS_BOTTOM_LEFT)
            : (transform->m_data.transform.flip_y
               ? UI_SPRITE_2D_TRANSFORM_POS_TOP_RIGHT
               : UI_SPRITE_2D_TRANSFORM_POS_BOTTOM_RIGHT);
        break;
    default:
        break;
    }

    switch(pos_policy) {
    case UI_SPRITE_2D_TRANSFORM_POS_ORIGIN:
        r.x = 0;
        r.y = 0;
        return r;
    case UI_SPRITE_2D_TRANSFORM_POS_TOP_LEFT:
        r.x = transform->m_data.transform.rect.lt.x;
        r.y = transform->m_data.transform.rect.lt.y;
        break;
    case UI_SPRITE_2D_TRANSFORM_POS_TOP_CENTER:
        r.x = (transform->m_data.transform.rect.lt.x + transform->m_data.transform.rect.rb.x) / 2.0f;
        r.y = transform->m_data.transform.rect.lt.y;
        break;
    case UI_SPRITE_2D_TRANSFORM_POS_TOP_RIGHT:
        r.x = transform->m_data.transform.rect.rb.x;
        r.y = transform->m_data.transform.rect.lt.y;
        break;
    case UI_SPRITE_2D_TRANSFORM_POS_CENTER_LEFT:
        r.x = transform->m_data.transform.rect.lt.x;
        r.y = (transform->m_data.transform.rect.lt.y + transform->m_data.transform.rect.rb.y) / 2.0f;
        break;
    case UI_SPRITE_2D_TRANSFORM_POS_CENTER:
        r.x = (transform->m_data.transform.rect.lt.x + transform->m_data.transform.rect.rb.x) / 2.0f;
        r.y = (transform->m_data.transform.rect.lt.y + transform->m_data.transform.rect.rb.y) / 2.0f;
        break;
    case UI_SPRITE_2D_TRANSFORM_POS_CENTER_RIGHT:
        r.x = transform->m_data.transform.rect.rb.x;
        r.y = (transform->m_data.transform.rect.lt.y + transform->m_data.transform.rect.rb.y) / 2.0f;
        break;
    case UI_SPRITE_2D_TRANSFORM_POS_BOTTOM_LEFT:
        r.x = transform->m_data.transform.rect.lt.x;
        r.y = transform->m_data.transform.rect.rb.y;
        break;
    case UI_SPRITE_2D_TRANSFORM_POS_BOTTOM_CENTER:
        r.x = (transform->m_data.transform.rect.lt.x + transform->m_data.transform.rect.rb.x) / 2.0f;
        r.y = transform->m_data.transform.rect.rb.y;
        break;
    case UI_SPRITE_2D_TRANSFORM_POS_BOTTOM_RIGHT:
        r.x = transform->m_data.transform.rect.rb.x;
        r.y = transform->m_data.transform.rect.rb.y;
        break;
    default: {
        ui_sprite_component_t component = ui_sprite_component_from_data(transform);
        ui_sprite_entity_t entity = ui_sprite_component_entity(component);

        CPE_ERROR(
            transform->m_module->m_em, "entity %d(%s): transform: get pos policy %d is unknown",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), pos_policy);

        r.x = 0.0f;
        r.y = 0.0f;
        return r;
    }
    }

    if (adj_type) {
        r = ui_sprite_2d_transform_adj_local_pos(transform, r, adj_type);
    }

    return r;
}

UI_SPRITE_2D_PAIR ui_sprite_2d_transform_world_pos(ui_sprite_2d_transform_t transform, uint8_t pos_policy, uint8_t adj_type) {
    UI_SPRITE_2D_PAIR r = ui_sprite_2d_transform_local_pos(transform, pos_policy, adj_type);

    r.x += transform->m_data.transform.pos.x;
    r.y += transform->m_data.transform.pos.y;

    return r;
}

UI_SPRITE_2D_PAIR ui_sprite_2d_transform_origin_pos(ui_sprite_2d_transform_t transform) {
    return transform->m_data.transform.pos;
}

void ui_sprite_2d_transform_set_origin_pos(ui_sprite_2d_transform_t transform, UI_SPRITE_2D_PAIR pos) {
    if (pos.x != transform->m_data.transform.pos.x || pos.y != transform->m_data.transform.pos.y) {
        ui_sprite_component_t component = ui_sprite_component_from_data(transform);
        ui_sprite_entity_t entity = ui_sprite_component_entity(component);

        transform->m_data.transform.pos = pos;

        if (ui_sprite_component_is_active(component)) {
            ui_sprite_entity_notify_attr_updated(entity, "transform.pos");
        }
    }
}

UI_SPRITE_2D_PAIR ui_sprite_2d_transform_scale_pair(ui_sprite_2d_transform_t transform) {
    return transform->m_data.transform.scale;
}

float ui_sprite_2d_transform_scale(ui_sprite_2d_transform_t transform) {
    return
        transform->m_data.transform.scale.x > transform->m_data.transform.scale.y
        ? transform->m_data.transform.scale.x
        : transform->m_data.transform.scale.y;
}

void ui_sprite_2d_transform_set_scale_pair(ui_sprite_2d_transform_t transform, UI_SPRITE_2D_PAIR scale) {
    if (scale.x != transform->m_data.transform.scale.x || scale.y != transform->m_data.transform.scale.y) {
        ui_sprite_component_t component = ui_sprite_component_from_data(transform);
        ui_sprite_entity_t entity = ui_sprite_component_entity(component);

        transform->m_data.transform.scale = scale;

        if (ui_sprite_component_is_active(component)) {
            ui_sprite_entity_notify_attr_updated(entity, "transform.scale");
        }
    }
}

void ui_sprite_2d_transform_set_scale(ui_sprite_2d_transform_t transform, float scale) {
    UI_SPRITE_2D_PAIR scale_pair = { scale, scale };
    ui_sprite_2d_transform_set_scale_pair(transform, scale_pair);
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

void ui_sprite_2d_transform_merge_rect(ui_sprite_2d_transform_t transform, UI_SPRITE_2D_RECT const * rect) {
    if (ui_sprite_2d_rect_merge(&transform->m_data.transform.rect, rect) != 0) {
        ui_sprite_component_t component = ui_sprite_component_from_data(transform);
        ui_sprite_entity_t entity = ui_sprite_component_entity(component);

        if (ui_sprite_component_is_active(component)) {
            ui_sprite_entity_notify_attr_updated(entity, "transform.rect");
        }
    }
}

UI_SPRITE_2D_RECT const * ui_sprite_2d_transform_rect(ui_sprite_2d_transform_t transform) {
    return &transform->m_data.transform.rect;
}

uint8_t ui_sprite_2d_transform_flip_x(ui_sprite_2d_transform_t transform) {
    return transform->m_data.transform.flip_x;
}

uint8_t ui_sprite_2d_transform_flip_y(ui_sprite_2d_transform_t transform) {
    return transform->m_data.transform.flip_y;
}

void ui_sprite_2d_transform_set_flip(ui_sprite_2d_transform_t transform, uint8_t flip_x, uint8_t flip_y) {
    assert(flip_x == 0 || flip_x == 1);
    assert(flip_y == 0 || flip_y == 1);

    if (transform->m_data.transform.flip_x != flip_x) {
        ui_sprite_component_t component = ui_sprite_component_from_data(transform);
        ui_sprite_entity_t entity = ui_sprite_component_entity(component);

        transform->m_data.transform.flip_x = flip_x;

        if (ui_sprite_component_is_active(component)) {
            ui_sprite_entity_notify_attr_updated(entity, "transform.flip_x");
        }
    }

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

float ui_sprite_2d_transform_adj_angle_by_flip(ui_sprite_2d_transform_t transform, float angle) {
    assert(angle >= -180.f && angle <= 180.f);

    if (transform->m_data.transform.flip_x) {
		angle = cpe_math_angle_regular(180.0f - angle);
    }

    if (transform->m_data.transform.flip_y) {
		angle = cpe_math_angle_regular(0.0 - angle);
    }

    return angle;
}

float ui_sprite_2d_transform_adj_radians_by_flip(ui_sprite_2d_transform_t transform, float radians) {
	assert(radians >= -M_PI && radians <= M_PI);
	if (transform->m_data.transform.flip_x) {
		radians = cpe_math_radians_diff(M_PI, radians);
	}

	if (transform->m_data.transform.flip_y) {
		radians = cpe_math_radians_diff(0, radians);
	}

    return radians;
}

UI_SPRITE_2D_PAIR
ui_sprite_2d_transform_adj_world_pos(ui_sprite_2d_transform_t transform, UI_SPRITE_2D_PAIR pos, uint8_t adj_type)
{
    pos.x -= transform->m_data.transform.pos.x;
    pos.y -= transform->m_data.transform.pos.y;

    pos = ui_sprite_2d_transform_adj_local_pos(transform, pos, adj_type);

    pos.x += transform->m_data.transform.pos.x;
    pos.y += transform->m_data.transform.pos.y;

    return pos;
}

UI_SPRITE_2D_PAIR
ui_sprite_2d_transform_adj_local_pos(ui_sprite_2d_transform_t transform, UI_SPRITE_2D_PAIR pos, uint8_t adj_type) {
    if (adj_type & UI_SPRITE_2D_TRANSFORM_POS_ADJ_BY_FLIP) {
        if (transform->m_data.transform.flip_x) pos.x *= -1.0f;
        if (transform->m_data.transform.flip_y) pos.y *= -1.0f;
    }

    if (adj_type & UI_SPRITE_2D_TRANSFORM_POS_ADJ_BY_SCALE) {
        pos.x *= transform->m_data.transform.scale.x;
        pos.y *= transform->m_data.transform.scale.y;
    }

    if (adj_type & UI_SPRITE_2D_TRANSFORM_POS_ADJ_BY_ANGLE && transform->m_data.transform.angle) {
        float distance = cpe_math_distance(0, 0, pos.x, pos.y);
        float radians = cpe_math_radians(0, 0, pos.x, pos.y);

        radians = cpe_math_radians_add(radians, cpe_math_angle_to_radians(transform->m_data.transform.angle));

        pos.x = distance * cos(radians);
        pos.y = distance * sin(radians);
    }

    return pos;
}

UI_SPRITE_2D_PAIR ui_sprite_2d_transform_world_to_local(ui_sprite_2d_transform_t transform, UI_SPRITE_2D_PAIR world_pt) {
    UI_SPRITE_2D_PAIR r;
    float distance;
    float radians;

    if (fabs(transform->m_data.transform.pos.y - world_pt.y) < 0.01f) {
        r.x = world_pt.x - transform->m_data.transform.pos.x;
        r.y = transform->m_data.transform.pos.y;
    }
    else {
        assert(transform->m_data.transform.scale.x > 0.0f && transform->m_data.transform.scale.y > 0.0f);
        
        distance =
            cpe_math_distance(
                transform->m_data.transform.pos.x, transform->m_data.transform.pos.y,
                world_pt.x, world_pt.y);

        radians = 
            distance > 0.01f
            ? cpe_math_radians(
                transform->m_data.transform.pos.x, transform->m_data.transform.pos.y,
                world_pt.x, world_pt.y)
            : 0.0f;

        radians = ui_sprite_2d_transform_adj_radians_by_flip(transform, radians);

        r.x = distance * cos(radians);
        r.y = distance * sin(radians);
    }

    r.x /= transform->m_data.transform.scale.x;
    r.y /= transform->m_data.transform.scale.y;

    return r;
}

UI_SPRITE_2D_PAIR ui_sprite_2d_transform_local_to_world(ui_sprite_2d_transform_t transform, UI_SPRITE_2D_PAIR local_pt) {
    UI_SPRITE_2D_PAIR r;
    float distance;
    float radians;

    assert(transform->m_data.transform.scale.x > 0.0f && transform->m_data.transform.scale.y > 0.0f);

    distance = cpe_math_distance(0, 0, local_pt.x, local_pt.y);

    radians = distance > 0.01f ? cpe_math_radians(0, 0, local_pt.x, local_pt.y) : 0.0f;

    radians = cpe_math_radians_diff(radians, cpe_math_angle_to_radians(transform->m_data.transform.angle));

    radians = ui_sprite_2d_transform_adj_radians_by_flip(transform, radians);

    r.x = transform->m_data.transform.pos.x + distance * cos(radians) * transform->m_data.transform.scale.x;
    r.y = transform->m_data.transform.pos.y + distance * sin(radians) * transform->m_data.transform.scale.y;

    return r;
}

const char * UI_SPRITE_2D_TRANSFORM_NAME = "Transform";
