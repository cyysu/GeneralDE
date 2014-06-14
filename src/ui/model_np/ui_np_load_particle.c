#include <assert.h>
#include <math.h>
#include "libxml/xmlstring.h"
#include "libxml/parser.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/dr/dr_data.h"
#include "ui/model/ui_data_particle.h"
#include "ui/model/ui_data_src.h"
#include "ui/model/ui_data_mgr.h"
#include "ui_np_utils.h"
#include "ui_np_load_utils.h"

struct ui_np_load_particle_ctx {
    ui_data_mgr_t m_mgr;
    ui_data_src_t m_src;
    error_monitor_t m_em;
    ui_data_particle_t m_particle;
    ui_data_particle_emitter_t m_cur_emitter;
    UI_PARTICLE_EMITTER * m_emitter_data;
    ui_data_particle_mod_t m_cur_mod;
    UI_PARTICLE_MOD * m_mod_data;
    struct {
        UI_PARTICLE_MOD_ACCEL_ATTRACT * m_accel_attract;
        UI_PARTICLE_MOD_ACCEL_DAMPING * m_accel_damping;
        UI_PARTICLE_MOD_ACCEL_SEED * m_accel_seed;
        UI_PARTICLE_MOD_ACCEL_SINE * m_accel_sine;
        UI_PARTICLE_MOD_COLOR_CURVED * m_color_curved;
        UI_PARTICLE_MOD_COLOR_CURVED_ALPHA * m_color_curved_alpha;
        UI_PARTICLE_MOD_COLOR_FIXED * m_color_fixed;
        UI_PARTICLE_MOD_COLOR_OVER_LIFE * m_color_over_life;
        UI_PARTICLE_MOD_COLOR_SEED * m_color_seed;
        UI_PARTICLE_MOD_LIFETIME_SEED * m_lifetime_seed;
        UI_PARTICLE_MOD_LOCATION_ORBIT * m_location_orbit;
        UI_PARTICLE_MOD_LOCATION_SEED * m_location_seed;
        UI_PARTICLE_MOD_ROTATION2D_SEED * m_rotation2d_seed;
        UI_PARTICLE_MOD_SIZE_CURVED_UNIFORM * m_size_curved_uniform;
        UI_PARTICLE_MOD_SIZE_CURVED * m_size_curved;
        UI_PARTICLE_MOD_SIZE_UNIFORM_OVER_LIFE * m_size_uniform_over_life;
        UI_PARTICLE_MOD_SIZE_OVER_LIFE * m_size_over_life;
        UI_PARTICLE_MOD_SIZE_SEED * m_size_seed;
        UI_PARTICLE_MOD_SIZE_UNIFORM_SEED * m_size_uniform_seed;
        UI_PARTICLE_MOD_TEXCOORD_FLIPBOOK_UV * m_texcoord_flipbook_uv;
        UI_PARTICLE_MOD_TEXCOORD_SCROLL_ANIM * m_texcoord_scroll_anim;
        UI_PARTICLE_MOD_TEXCOORD_TILE_SUB_TEX * m_texcoord_tile_sub_tex;
        UI_PARTICLE_MOD_UBER_CIRCLE_SPAWN * m_uber_circle_spawn;
        UI_PARTICLE_MOD_UBER_ELLIPSE_SPAWN * m_uber_ellipse_spawn;
        UI_PARTICLE_MOD_VELOCITY_ATTRACT * m_velocity_attract;
        UI_PARTICLE_MOD_VELOCITY_SEED * m_velocity_seed;
        UI_PARTICLE_MOD_VELOCITY_THRESHOLD_STOP * m_velocity_threshold_stop;
    } m_mod;
    UI_CURVE_CHANEL * m_curve_chanel;
    char m_cur_tag_name[64];
};

static void ui_np_load_particle_startElement(
        void* inputCtx,
        const xmlChar* localname,
        const xmlChar* prefix,
        const xmlChar* URI,
        int nb_namespaces,
        const xmlChar** namespaces,
        int nb_attributes,
        int nb_defaulted,
        const xmlChar** attributes)
{
    struct ui_np_load_particle_ctx * ctx = (struct ui_np_load_particle_ctx *)(inputCtx);

    if (strcmp((const char *)localname, "NPParticleSpriteSRC") == 0) {
        ctx->m_emitter_data = NULL;
        ctx->m_cur_mod = NULL;
        ctx->m_mod_data = NULL;
        ctx->m_curve_chanel = NULL;

        ctx->m_cur_emitter = ui_data_particle_emitter_create(ctx->m_particle);
        if (ctx->m_cur_emitter == NULL) {
            CPE_ERROR(ctx->m_em, "create particle emitter: create fail!");
            return;
        }

        ctx->m_emitter_data = ui_data_particle_emitter_data(ctx->m_cur_emitter);
        assert(ctx->m_emitter_data);;

        dr_meta_set_defaults(
            ctx->m_emitter_data, sizeof(*ctx->m_emitter_data), ui_data_particle_emitter_meta(ctx->m_mgr),
            DR_SET_DEFAULTS_POLICY_NO_DEFAULT_IGNORE);
    }
    else if (strcmp((const char *)localname, "MOD") == 0) {
        char mod_type_name[64];
        uint32_t mod_type;

        ctx->m_cur_mod = NULL;
        ctx->m_mod_data = NULL;
        bzero(&ctx->m_mod, sizeof(ctx->m_mod));

        if (ctx->m_cur_emitter == NULL) return;

        UI_NP_XML_READ_ATTR_STRING(mod_type_name, "Type");

        mod_type = ui_data_np_particle_mod_type(mod_type_name);
        if (mod_type == 0) {
            CPE_ERROR(ctx->m_em, "create particle_mod: type %s unknown!", mod_type_name);
            return;
        }

        ctx->m_cur_mod = ui_data_particle_mod_create(ctx->m_cur_emitter);
        if (ctx->m_cur_mod  == NULL) {
            CPE_ERROR(ctx->m_em, "create mod fail!");
            return;
        }
        ctx->m_mod_data = ui_data_particle_mod_data(ctx->m_cur_mod);
        assert(ctx->m_mod_data);

        ctx->m_mod_data->type = mod_type;

        switch(ctx->m_mod_data->type) {
        case ui_particle_mod_type_accel_attract:
            ctx->m_mod.m_accel_attract = &ctx->m_mod_data->data.accel_attract;
            break;
        case ui_particle_mod_type_accel_damping:
            ctx->m_mod.m_accel_damping = &ctx->m_mod_data->data.accel_damping;
            break;
        case ui_particle_mod_type_accel_seed:
            ctx->m_mod.m_accel_seed = &ctx->m_mod_data->data.accel_seed;
            break;
        case ui_particle_mod_type_accel_sine:
            ctx->m_mod.m_accel_sine = &ctx->m_mod_data->data.accel_sine;
            break;
        case ui_particle_mod_type_color_curved:
            ctx->m_mod.m_color_curved = &ctx->m_mod_data->data.color_curved;
            break;
        case ui_particle_mod_type_color_curved_alpha:
            ctx->m_mod.m_color_curved_alpha = &ctx->m_mod_data->data.color_curved_alpha;
            break;
        case ui_particle_mod_type_color_fixed:
            ctx->m_mod.m_color_fixed = &ctx->m_mod_data->data.color_fixed;
            break;
        case ui_particle_mod_type_color_over_life:
            ctx->m_mod.m_color_over_life = &ctx->m_mod_data->data.color_over_life;
            break;
        case ui_particle_mod_type_color_seed:
            ctx->m_mod.m_color_seed = &ctx->m_mod_data->data.color_seed;
            break;
        case ui_particle_mod_type_lifetime_seed:
            ctx->m_mod.m_lifetime_seed = &ctx->m_mod_data->data.lifetime_seed;
            break;
        case ui_particle_mod_type_location_orbit:
            ctx->m_mod.m_location_orbit = &ctx->m_mod_data->data.location_orbit;
            break;
        case ui_particle_mod_type_location_seed:
            ctx->m_mod.m_location_seed = &ctx->m_mod_data->data.location_seed;
            break;
        case ui_particle_mod_type_rotation2d_seed:
            ctx->m_mod.m_rotation2d_seed = &ctx->m_mod_data->data.rotation2d_seed;
            break;
        case ui_particle_mod_type_size_curved_uniform:
            ctx->m_mod.m_size_curved_uniform = &ctx->m_mod_data->data.size_curved_uniform;
            break;
        case ui_particle_mod_type_size_uniform_over_life:
            ctx->m_mod.m_size_uniform_over_life = &ctx->m_mod_data->data.size_uniform_over_life;
            break;
        case ui_particle_mod_type_size_curved:
            ctx->m_mod.m_size_curved = &ctx->m_mod_data->data.size_curved;
            break;
        case ui_particle_mod_type_size_over_life:
            ctx->m_mod.m_size_over_life = &ctx->m_mod_data->data.size_over_life;
            break;
        case ui_particle_mod_type_size_seed:
            ctx->m_mod.m_size_seed = &ctx->m_mod_data->data.size_seed;
            break;
        case ui_particle_mod_type_size_uniform_seed:
            ctx->m_mod.m_size_uniform_seed = &ctx->m_mod_data->data.size_uniform_seed;
            break;
        case ui_particle_mod_type_texcoord_flipbook_uv:
            ctx->m_mod.m_texcoord_flipbook_uv = &ctx->m_mod_data->data.texcoord_flipbook_uv;
            break;
        case ui_particle_mod_type_texcoord_scroll_anim:
            ctx->m_mod.m_texcoord_scroll_anim = &ctx->m_mod_data->data.texcoord_scroll_anim;
            break;
        case ui_particle_mod_type_texcoord_tile_sub_tex:
            ctx->m_mod.m_texcoord_tile_sub_tex = &ctx->m_mod_data->data.texcoord_tile_sub_tex;
            break;
        case ui_particle_mod_type_uber_circle_spawn:
            ctx->m_mod.m_uber_circle_spawn = &ctx->m_mod_data->data.uber_circle_spawn;
            break;
        case ui_particle_mod_type_uber_ellipse_spawn:
            ctx->m_mod.m_uber_ellipse_spawn = &ctx->m_mod_data->data.uber_ellipse_spawn;
            break;
        case ui_particle_mod_type_velocity_attract:
            ctx->m_mod.m_velocity_attract = &ctx->m_mod_data->data.velocity_attract;
            break;
        case ui_particle_mod_type_velocity_seed:
            ctx->m_mod.m_velocity_seed = &ctx->m_mod_data->data.velocity_seed;
            break;
        case ui_particle_mod_type_velocity_threshold_stop:
            ctx->m_mod.m_velocity_threshold_stop = &ctx->m_mod_data->data.velocity_threshold_stop;
            break;
        }
    }
    else if (strcmp((const char *)localname, "CurveA") == 0) {
        if (ctx->m_mod.m_color_curved_alpha) {
            ctx->m_curve_chanel = &ctx->m_mod.m_color_curved_alpha->chanel_a;
        }
    }
    else if (strcmp((const char *)localname, "CurveKey") == 0) {
        if (ctx->m_curve_chanel) {
            UI_CURVE_POINT * point;
            float angle_l, angle_r;

            if (ctx->m_curve_chanel->point_count + 1 > CPE_ARRAY_SIZE(ctx->m_curve_chanel->points)) {
                CPE_ERROR(ctx->m_em, "curved chanel overflow!");
                return;
            }

            point = ctx->m_curve_chanel->points + ctx->m_curve_chanel->point_count++;

            UI_NP_XML_READ_ATTR_INT(uint8_t, point->interp, "Mode");
            UI_NP_XML_READ_ATTR_FLOAT(point->key, "Key");
            UI_NP_XML_READ_ATTR_FLOAT(point->ret, "Ret");

            UI_NP_XML_READ_ATTR_FLOAT(angle_l, "Enter");
            UI_NP_XML_READ_ATTR_FLOAT(angle_r, "Leave");

            point->enter_tan = tan(angle_l / 180.0f * M_PI);
            point->leave_tan = tan(angle_r / 180.0f * M_PI);
        }
    }
    else {
        strncpy(ctx->m_cur_tag_name, (const char*)localname, sizeof(ctx->m_cur_tag_name));
    }
}

static void ui_np_load_particle_endElement(
        void* inputCtx,
        const xmlChar* localname,
        const xmlChar* prefix,
        const xmlChar* URI)
{
    struct ui_np_load_particle_ctx * ctx = (struct ui_np_load_particle_ctx *)(inputCtx);
    ctx->m_cur_tag_name[0] = 0;

    if (strcmp((const char *)localname, "MOD") == 0) {
        ctx->m_cur_mod = NULL;
        ctx->m_mod_data = NULL;
        ctx->m_curve_chanel = NULL;
    }
    else if (strcmp((const char *)localname, "NPParticleSpriteSRC") == 0) {
        ctx->m_cur_emitter = NULL;
        ctx->m_emitter_data = NULL;
        ctx->m_cur_mod = NULL;
        ctx->m_mod_data = NULL;
        ctx->m_curve_chanel = NULL;
    }
    else if (strcmp((const char *)localname, "CurveA") == 0) {
        ctx->m_curve_chanel = NULL;
    }
}

static void ui_np_load_particle_characters(void * inputCtx, const xmlChar *ch, int len) {
    struct ui_np_load_particle_ctx * ctx = (struct ui_np_load_particle_ctx *)(inputCtx);

    if (strcmp(ctx->m_cur_tag_name, "SpawnRate") == 0) {
        if (ctx->m_emitter_data) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_emitter_data->spawn_rate);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxAmount") == 0) {
        if (ctx->m_emitter_data) {
            UI_NP_XML_READ_VALUE_INT(uint32_t, ctx->m_emitter_data->max_amount);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "Name") == 0) {
        if (ctx->m_mod_data) {
            UI_NP_XML_READ_VALUE_STRING(ctx->m_mod_data->name);
        }
        else if (ctx->m_emitter_data) {
            UI_NP_XML_READ_VALUE_STRING(ctx->m_emitter_data->name);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "XFormMode") == 0) {
        if (ctx->m_emitter_data) {
            UI_NP_XML_READ_VALUE_INT(uint8_t, ctx->m_emitter_data->xform_mod);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "BoundMode") == 0) {
        if (ctx->m_emitter_data) {
            UI_NP_XML_READ_VALUE_INT(uint8_t, ctx->m_emitter_data->bound_mod);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "AutoUpDir") == 0) {
        if (ctx->m_emitter_data) {
            UI_NP_XML_READ_VALUE_BOOL(ctx->m_emitter_data->auto_up_dir);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "InitDelayTime") == 0) {
        if (ctx->m_emitter_data) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_emitter_data->init_delay_time);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "LoopDelayTime") == 0) {
        if (ctx->m_emitter_data) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_emitter_data->loop_delay_time);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "Duration") == 0) {
        if (ctx->m_emitter_data) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_emitter_data->duration);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "TimeScale") == 0) {
        if (ctx->m_emitter_data) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_emitter_data->time_scale);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "RepeatTimes") == 0) {
        if (ctx->m_emitter_data) {
            UI_NP_XML_READ_VALUE_INT(uint32_t, ctx->m_emitter_data->repeat_time);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxExtraBrust") == 0) {
        if (ctx->m_emitter_data) {
            UI_NP_XML_READ_VALUE_INT(uint32_t, ctx->m_emitter_data->max_extra_brust);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MinExtraBrust") == 0) {
        if (ctx->m_emitter_data) {
            UI_NP_XML_READ_VALUE_INT(uint32_t, ctx->m_emitter_data->min_extra_brust);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "Scale") == 0) {
        if (ctx->m_emitter_data) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_emitter_data->scale);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "ChildFxFile") == 0) {
        if (ctx->m_emitter_data) {
            UI_NP_XML_READ_VALUE_STRING(ctx->m_emitter_data->child_fx_file);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "WaitChildFX") == 0) {
        if (ctx->m_emitter_data) {
            UI_NP_XML_READ_VALUE_BOOL(ctx->m_emitter_data->wait_child_fx);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "PassOnColor") == 0) {
        if (ctx->m_emitter_data) {
            UI_NP_XML_READ_VALUE_BOOL(ctx->m_emitter_data->pass_on_color);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "TilingU") == 0) {
        if (ctx->m_emitter_data) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_emitter_data->tiling_u);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "TilingV") == 0) {
        if (ctx->m_emitter_data) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_emitter_data->tiling_v);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "BlendMode") == 0) {
        if (ctx->m_emitter_data) {
            UI_NP_XML_READ_VALUE_INT(uint8_t, ctx->m_emitter_data->blend_mode);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "TexAtlasBiasX") == 0) {
        if (ctx->m_emitter_data) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_emitter_data->atlas_x);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "TexAtlasBiasY") == 0) {
        if (ctx->m_emitter_data) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_emitter_data->atlas_y);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "TexAtlasSizeX") == 0) {
        if (ctx->m_emitter_data) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_emitter_data->atlas_w);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "TexAtlasSizeY") == 0) {
        if (ctx->m_emitter_data) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_emitter_data->atlas_h);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "Origin") == 0) {
        if (ctx->m_emitter_data) {
            UI_NP_XML_READ_VALUE_INT(uint8_t, ctx->m_emitter_data->origin);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "BaseTexel") == 0) {
        if (ctx->m_emitter_data) {
            UI_NP_XML_READ_VALUE_STRING(ctx->m_emitter_data->texture);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MinBaseAccelX") == 0) {
        if (ctx->m_mod.m_accel_seed) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_accel_seed->min_base.value[0]);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MinBaseAccelY") == 0) {
        if (ctx->m_mod.m_accel_seed) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_accel_seed->min_base.value[1]);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MinBaseAccelZ") == 0) {
        if (ctx->m_mod.m_accel_seed) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_accel_seed->min_base.value[2]);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxBaseAccelX") == 0) {
        if (ctx->m_mod.m_accel_seed) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_accel_seed->max_base.value[0]);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxBaseAccelY") == 0) {
        if (ctx->m_mod.m_accel_seed) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_accel_seed->max_base.value[1]);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxBaseAccelZ") == 0) {
        if (ctx->m_mod.m_accel_seed) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_accel_seed->max_base.value[2]);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MinBaseSize") == 0) {
        if (ctx->m_mod.m_size_uniform_seed) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_size_uniform_seed->min_base_size);
        }
        else if (ctx->m_mod.m_size_uniform_over_life) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_size_uniform_over_life->min_base_size);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxBaseSize") == 0) {
        if (ctx->m_mod.m_size_uniform_seed) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_size_uniform_seed->max_base_size);
        }
        else if (ctx->m_mod.m_size_uniform_over_life) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_size_uniform_over_life->max_base_size);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MinBaseLifetime") == 0) {
        if (ctx->m_mod.m_lifetime_seed) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_lifetime_seed->min_base_time);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxBaseLifetime") == 0) {
        if (ctx->m_mod.m_lifetime_seed) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_lifetime_seed->max_base_time);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "FadeBeginTime") == 0) {
        if (ctx->m_mod.m_size_uniform_over_life) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_size_uniform_over_life->fade_begin_time);
        }
        if (ctx->m_mod.m_color_over_life) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_color_over_life->fade_begin_time);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MinBaseLocationX") == 0) {
        if (ctx->m_mod.m_location_seed) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_location_seed->min_base_location.value[0]);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MinBaseLocationY") == 0) {
        if (ctx->m_mod.m_location_seed) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_location_seed->min_base_location.value[1]);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MinBaseLocationZ") == 0) {
        if (ctx->m_mod.m_location_seed) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_location_seed->min_base_location.value[2]);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxBaseLocationX") == 0) {
        if (ctx->m_mod.m_location_seed) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_location_seed->max_base_location.value[0]);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxBaseLocationY") == 0) {
        if (ctx->m_mod.m_location_seed) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_location_seed->max_base_location.value[1]);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxBaseLocationZ") == 0) {
        if (ctx->m_mod.m_location_seed) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_location_seed->max_base_location.value[2]);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "AngleDelta") == 0) {
        if (ctx->m_mod.m_uber_circle_spawn) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_uber_circle_spawn->angle_delta);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "CircleRadius") == 0) {
        if (ctx->m_mod.m_uber_circle_spawn) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_uber_circle_spawn->circle_radius);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MinRadialAccel") == 0) {
        if (ctx->m_mod.m_uber_circle_spawn) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_uber_circle_spawn->min_radial_accel);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxRadialAccel") == 0) {
        if (ctx->m_mod.m_uber_circle_spawn) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_uber_circle_spawn->max_radial_accel);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MinTangentAccel") == 0) {
        if (ctx->m_mod.m_uber_circle_spawn) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_uber_circle_spawn->min_tangent_accel);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxTangentAccel") == 0) {
        if (ctx->m_mod.m_uber_circle_spawn) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_uber_circle_spawn->max_tangent_accel);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MinRadialVelocity") == 0) {
        if (ctx->m_mod.m_uber_circle_spawn) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_uber_circle_spawn->min_radial_velocity);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxRadialVelocity") == 0) {
        if (ctx->m_mod.m_uber_circle_spawn) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_uber_circle_spawn->max_radial_velocity);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MinArcAngle") == 0) {
        if (ctx->m_mod.m_uber_circle_spawn) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_uber_circle_spawn->min_arc_angle);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxArcAngle") == 0) {
        if (ctx->m_mod.m_uber_circle_spawn) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_uber_circle_spawn->max_arc_angle);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "DistributeDelta") == 0) {
        if (ctx->m_mod.m_uber_circle_spawn) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_uber_circle_spawn->distribute_delta);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "InitialAngle") == 0) {
        if (ctx->m_mod.m_uber_circle_spawn) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_uber_circle_spawn->initial_angle);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "Framerate") == 0) {
        if (ctx->m_mod.m_texcoord_flipbook_uv) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_texcoord_flipbook_uv->frame_rate);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "Loop") == 0) {
        if (ctx->m_mod.m_texcoord_flipbook_uv) {
            UI_NP_XML_READ_VALUE_BOOL(ctx->m_mod.m_texcoord_flipbook_uv->loop);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "StartTile") == 0) {
        if (ctx->m_mod.m_texcoord_flipbook_uv) {
            UI_NP_XML_READ_VALUE_INT(int32_t, ctx->m_mod.m_texcoord_flipbook_uv->start_tile_index);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MinSpinRate") == 0) {
        if (ctx->m_mod.m_rotation2d_seed) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_rotation2d_seed->min_spin_rate);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxSpinRate") == 0) {
        if (ctx->m_mod.m_rotation2d_seed) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_rotation2d_seed->max_spin_rate);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MinInitSpin") == 0) {
        if (ctx->m_mod.m_rotation2d_seed) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_rotation2d_seed->min_init_spin);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxInitSpin") == 0) {
        if (ctx->m_mod.m_rotation2d_seed) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_rotation2d_seed->max_init_spin);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MinBaseSizeX") == 0) {
        if (ctx->m_mod.m_size_seed) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_size_seed->min_base_size.value[0]);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MinBaseSizeY") == 0) {
        if (ctx->m_mod.m_size_seed) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_size_seed->min_base_size.value[1]);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxBaseSizeX") == 0) {
        if (ctx->m_mod.m_size_seed) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_size_seed->max_base_size.value[0]);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxBaseSizeY") == 0) {
        if (ctx->m_mod.m_size_seed) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_size_seed->max_base_size.value[1]);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MinBaseColorA") == 0) {
        if (ctx->m_mod.m_color_over_life) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_color_over_life->min_base_color.a);
        }
        if (ctx->m_mod.m_color_seed) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_color_seed->min_base_color.a);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MinBaseColorR") == 0) {
        if (ctx->m_mod.m_color_over_life) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_color_over_life->min_base_color.r);
        }
        if (ctx->m_mod.m_color_seed) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_color_seed->min_base_color.r);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MinBaseColorG") == 0) {
        if (ctx->m_mod.m_color_over_life) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_color_over_life->min_base_color.g);
        }
        if (ctx->m_mod.m_color_seed) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_color_seed->min_base_color.g);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MinBaseColorB") == 0) {
        if (ctx->m_mod.m_color_over_life) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_color_over_life->min_base_color.b);
        }
        if (ctx->m_mod.m_color_seed) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_color_seed->min_base_color.b);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxBaseColorA") == 0) {
        if (ctx->m_mod.m_color_over_life) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_color_over_life->max_base_color.a);
        }
        if (ctx->m_mod.m_color_seed) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_color_seed->max_base_color.a);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxBaseColorR") == 0) {
        if (ctx->m_mod.m_color_over_life) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_color_over_life->max_base_color.r);
        }
        if (ctx->m_mod.m_color_seed) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_color_seed->max_base_color.r);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxBaseColorG") == 0) {
        if (ctx->m_mod.m_color_over_life) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_color_over_life->max_base_color.g);
        }
        if (ctx->m_mod.m_color_seed) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_color_seed->max_base_color.g);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxBaseColorB") == 0) {
        if (ctx->m_mod.m_color_over_life) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_color_over_life->max_base_color.b);
        }
        if (ctx->m_mod.m_color_seed) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_color_seed->max_base_color.b);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MinBaseVelocityX") == 0) {
        if (ctx->m_mod.m_velocity_seed) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_velocity_seed->min_base_velocity.value[0]);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MinBaseVelocityY") == 0) {
        if (ctx->m_mod.m_velocity_seed) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_velocity_seed->min_base_velocity.value[1]);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MinBaseVelocityZ") == 0) {
        if (ctx->m_mod.m_velocity_seed) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_velocity_seed->min_base_velocity.value[2]);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxBaseVelocityX") == 0) {
        if (ctx->m_mod.m_velocity_seed) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_velocity_seed->max_base_velocity.value[0]);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxBaseVelocityY") == 0) {
        if (ctx->m_mod.m_velocity_seed) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_velocity_seed->max_base_velocity.value[1]);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxBaseVelocityZ") == 0) {
        if (ctx->m_mod.m_velocity_seed) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_velocity_seed->max_base_velocity.value[2]);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MinMultiplier") == 0) {
        if (ctx->m_mod.m_velocity_seed) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_velocity_seed->min_multiplier);
        }
    }
    else if (strcmp(ctx->m_cur_tag_name, "MaxMultiplier") == 0) {
        if (ctx->m_mod.m_velocity_seed) {
            UI_NP_XML_READ_VALUE_FLOAT(ctx->m_mod.m_velocity_seed->max_multiplier);
        }
    }
}

static void ui_np_load_particle_structed_error(void * inputCtx, xmlErrorPtr err) {
    struct ui_np_load_particle_ctx * ctx = (struct ui_np_load_particle_ctx *)(inputCtx);

    if (err->code == XML_ERR_DOCUMENT_END) {
        ((xmlParserCtxtPtr)err->ctxt)->wellFormed = 1;
        xmlCtxtResetLastError(err->ctxt);
    }
    else {
        CPE_ERROR_SET_LEVEL(
            ctx->m_em,
            err->level >= XML_ERR_ERROR ? CPE_EL_ERROR : CPE_EL_WARNING);

        CPE_ERROR_SET_LINE(ctx->m_em, err->line);

        cpe_error_do_notify(ctx->m_em, "(%d) %s", err->code, err->message);
    }
}

static xmlSAXHandler g_ui_np_load_particle_callbacks = {
    NULL /* internalSubsetSAXFunc internalSubset */
    , NULL /* isStandaloneSAXFunc isStandalone */
    , NULL /* hasInternalSubsetSAXFunc hasInternalSubset */
    , NULL /* hasExternalSubsetSAXFunc hasExternalSubset */
    , NULL /* resolveEntitySAXFunc resolveEntity */
    , NULL /* getEntitySAXFunc getEntity */
    , NULL /* entityDeclSAXFunc entityDecl */
    , NULL /* notationDeclSAXFunc notationDecl */
    , NULL /* attributeDeclSAXFunc attributeDecl */
    , NULL /* elementDeclSAXFunc elementDecl */
    , NULL /* unparsedEntityDeclSAXFunc unparsedEntityDecl */
    , NULL /* setDocumentLocatorSAXFunc setDocumentLocator */
    , NULL /* startDocumentSAXFunc startDocument */
    , NULL /* endDocumentSAXFunc endDocument */
    , NULL /* startElementSAXFunc startElement */
    , NULL /* endElementSAXFunc endElement */
    , NULL /* referenceSAXFunc reference */
    , ui_np_load_particle_characters /* charactersSAXFunc characters */
    , NULL /* ignorableWhitespaceSAXFunc ignorableWhitespace */
    , NULL /* processingInstructionSAXFunc processingInstruction */
    , NULL /* commentSAXFunc comment */
    , NULL /* warningSAXFunc warning */
    , NULL /* errorSAXFunc error */
    , NULL /* fatalErrorSAXFunc fatalError; unused error() get all the errors * */
    , NULL /* getParameterEntitySAXFunc getParameterEntity */
    , NULL /* cdataBlockSAXFunc cdataBlock */
    , NULL /* externalSubsetSAXFunc externalSubset */
    , XML_SAX2_MAGIC /* unsigned int initialized */
    , NULL /* void *_private */
    , ui_np_load_particle_startElement /* startElementNsSAX2Func startElementNs */
    , ui_np_load_particle_endElement /* endElementNsSAX2Func endElementNs */
    , ui_np_load_particle_structed_error /* xmlStructuredErrorFunc serror */
};

void ui_data_np_load_particle_i(void * p, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em) {
    struct ui_np_load_particle_ctx ctx;
    struct mem_buffer data_buff;
    char * data;
    char * data_end;
    const char * sep = "</NPParticleSpriteSRC>";

    bzero(&ctx, sizeof(ctx));
    ctx.m_mgr = mgr;
    ctx.m_src = src;
    ctx.m_em = em;

    mem_buffer_init(&data_buff, NULL);
    data = ui_data_np_load_src_to_buff(&data_buff, src, em);
    if (data == NULL) {
        mem_buffer_clear(&data_buff);
        return;
    }

    ctx.m_particle = ui_data_particle_create(mgr, src);
    if (ctx.m_particle == NULL) {
        CPE_ERROR(em, "np load particle: create particle fail");
        mem_buffer_clear(&data_buff);
        return;
    }

    while((data_end = strstr(data, sep))) {
        data_end += strlen(sep);

        if (xmlSAXUserParseMemory(&g_ui_np_load_particle_callbacks, &ctx, data, data_end - data) < 0) {
            CPE_ERROR(em, "np load particle: parse fail!");
        }

        data = data_end + 1;
    }

    mem_buffer_clear(&data_buff);
}

int ui_data_np_load_particle(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em) {
    int ret = 0;

    if (em) {
        CPE_DEF_ERROR_MONITOR_ADD(logError, em, cpe_error_save_last_errno, &ret);
        ui_data_np_load_particle_i(ctx, mgr, src, em);
        CPE_DEF_ERROR_MONITOR_REMOVE(logError, em);
    }
    else {
        CPE_DEF_ERROR_MONITOR(logError, cpe_error_save_last_errno, &ret);
        ui_data_np_load_particle_i(ctx, mgr, src, &logError);
    }

    return ret;
}
