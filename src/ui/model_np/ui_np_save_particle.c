#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/stream_file.h"
#include "ui/model/ui_data_particle.h"
#include "ui/model/ui_data_src.h"
#include "ui/model/ui_data_mgr.h"
#include "ui_np_save_utils.h"
#include "ui_np_utils.h"

static int ui_data_np_save_emitter_i(write_stream_t s, ui_data_particle_emitter_t emitter, error_monitor_t em) {
    struct ui_data_particle_mod_it particle_mod_it;
    ui_data_particle_mod_t particle_mod;
    UI_PARTICLE_EMITTER *  emitter_data = ui_data_particle_emitter_data(emitter);

    stream_printf(s, "<NPParticleSpriteSRC>\n");
    ui_data_np_save_float(s, 1, "SpawnRate", emitter_data->spawn_rate);
    ui_data_np_save_int(s, 1, "MaxAmount", (int)emitter_data->max_amount);

    if (emitter_data->name[0]) ui_data_np_save_str(s, 1, "Name", emitter_data->name);
    if (emitter_data->xform_mod != 0) ui_data_np_save_int(s, 1, "XFormMode", emitter_data->xform_mod);
    if (emitter_data->bound_mod != 0) ui_data_np_save_int(s, 1, "BoundMode", emitter_data->bound_mod);
    if (emitter_data->auto_up_dir != 0) ui_data_np_save_bool(s, 1, "AutoUpDir", emitter_data->auto_up_dir);
    if (emitter_data->init_delay_time != 0) ui_data_np_save_float(s, 1, "InitDelayTime", emitter_data->init_delay_time);
    if (emitter_data->loop_delay_time != 0) ui_data_np_save_float(s, 1, "LoopDelayTime", emitter_data->loop_delay_time);
    if (emitter_data->duration != 0) ui_data_np_save_float(s, 1, "Duration", emitter_data->duration);
    if (emitter_data->time_scale != 1.0f) ui_data_np_save_float(s, 1, "TimeScale", emitter_data->time_scale);
    if (emitter_data->repeat_time != 1) ui_data_np_save_int(s, 1, "RepeatTimes", emitter_data->repeat_time);
    if (emitter_data->max_extra_brust != 0) ui_data_np_save_int(s, 1, "MaxExtraBrust", emitter_data->max_extra_brust);
    if (emitter_data->min_extra_brust != 0) ui_data_np_save_int(s, 1, "MinExtraBrust", emitter_data->min_extra_brust);
    if (emitter_data->scale != 1.0f) ui_data_np_save_float(s, 1, "Scale", emitter_data->scale);
    if (emitter_data->child_fx_file[0]) ui_data_np_save_str(s, 1, "ChildFxFile", emitter_data->child_fx_file);
    if (emitter_data->wait_child_fx) ui_data_np_save_bool(s, 1, "WaitChildFX", emitter_data->wait_child_fx);
    if (emitter_data->pass_on_color) ui_data_np_save_bool(s, 1, "PassOnColor", emitter_data->pass_on_color);

    ui_data_particle_emitter_mods(&particle_mod_it, emitter);
    while((particle_mod = ui_data_particle_mod_it_next(&particle_mod_it))) {
        UI_PARTICLE_MOD const * particle_mod_data = ui_data_particle_mod_data(particle_mod);
        
        stream_printf(
            s, "    <MOD Type=\"%s\" Hash=\"%u\">\n",
            ui_data_np_particle_mod_type_name(particle_mod_data->type), 
            ui_data_np_particle_mod_type_hash(particle_mod_data->type));

        if (particle_mod_data->name[0]) ui_data_np_save_str(s, 2, "Name", particle_mod_data->name);

        switch(particle_mod_data->type) {
        case ui_particle_mod_type_accel_attract:
            break;
        case ui_particle_mod_type_accel_damping:
            break;
        case ui_particle_mod_type_accel_seed:
            ui_data_np_save_float(s, 2, "MinBaseAccelX", particle_mod_data->data.accel_seed.min_base.value[0]);
            ui_data_np_save_float(s, 2, "MinBaseAccelY", particle_mod_data->data.accel_seed.min_base.value[1]);
            ui_data_np_save_float(s, 2, "MinBaseAccelZ", particle_mod_data->data.accel_seed.min_base.value[2]);
            ui_data_np_save_float(s, 2, "MaxBaseAccelX", particle_mod_data->data.accel_seed.max_base.value[0]);
            ui_data_np_save_float(s, 2, "MaxBaseAccelY", particle_mod_data->data.accel_seed.max_base.value[1]);
            ui_data_np_save_float(s, 2, "MaxBaseAccelZ", particle_mod_data->data.accel_seed.max_base.value[2]);
            break;
        case ui_particle_mod_type_accel_sine:
            break;
        case ui_particle_mod_type_color_curved:
            ui_data_np_save_curve_chanel(s, 2, "CurveR", &particle_mod_data->data.color_curved.chanel_r);
            ui_data_np_save_curve_chanel(s, 2, "CurveG", &particle_mod_data->data.color_curved.chanel_g);
            ui_data_np_save_curve_chanel(s, 2, "CurveB", &particle_mod_data->data.color_curved.chanel_b);
            ui_data_np_save_curve_chanel(s, 2, "CurveA", &particle_mod_data->data.color_curved.chanel_a);
            break;
        case ui_particle_mod_type_color_curved_alpha:
            ui_data_np_save_curve_chanel(s, 2, "CurveA", &particle_mod_data->data.color_curved_alpha.chanel_a);
            break;
        case ui_particle_mod_type_color_fixed:
            break;
        case ui_particle_mod_type_color_over_life:
            ui_data_np_save_float(s, 2, "MinBaseColorA", particle_mod_data->data.color_over_life.min_base_color.a);
            ui_data_np_save_float(s, 2, "MinBaseColorR", particle_mod_data->data.color_over_life.min_base_color.r);
            ui_data_np_save_float(s, 2, "MinBaseColorG", particle_mod_data->data.color_over_life.min_base_color.g);
            ui_data_np_save_float(s, 2, "MinBaseColorB", particle_mod_data->data.color_over_life.min_base_color.b);
            ui_data_np_save_float(s, 2, "MaxBaseColorA", particle_mod_data->data.color_over_life.max_base_color.a);
            ui_data_np_save_float(s, 2, "MaxBaseColorR", particle_mod_data->data.color_over_life.max_base_color.r);
            ui_data_np_save_float(s, 2, "MaxBaseColorG", particle_mod_data->data.color_over_life.max_base_color.g);
            ui_data_np_save_float(s, 2, "MaxBaseColorB", particle_mod_data->data.color_over_life.max_base_color.b);
            ui_data_np_save_float(s, 2, "FadeBeginTime", particle_mod_data->data.color_over_life.fade_begin_time);
            break;
        case ui_particle_mod_type_color_seed:
            ui_data_np_save_float(s, 2, "MinBaseColorA", particle_mod_data->data.color_seed.min_base_color.a);
            ui_data_np_save_float(s, 2, "MinBaseColorR", particle_mod_data->data.color_seed.min_base_color.r);
            ui_data_np_save_float(s, 2, "MinBaseColorG", particle_mod_data->data.color_seed.min_base_color.g);
            ui_data_np_save_float(s, 2, "MinBaseColorB", particle_mod_data->data.color_seed.min_base_color.b);
            ui_data_np_save_float(s, 2, "MaxBaseColorA", particle_mod_data->data.color_seed.max_base_color.a);
            ui_data_np_save_float(s, 2, "MaxBaseColorR", particle_mod_data->data.color_seed.max_base_color.r);
            ui_data_np_save_float(s, 2, "MaxBaseColorG", particle_mod_data->data.color_seed.max_base_color.g);
            ui_data_np_save_float(s, 2, "MaxBaseColorB", particle_mod_data->data.color_seed.max_base_color.b);
            break;
        case ui_particle_mod_type_lifetime_seed:
            ui_data_np_save_float(s, 2, "MinBaseLifetime", particle_mod_data->data.lifetime_seed.min_base_time);
            ui_data_np_save_float(s, 2, "MaxBaseLifetime", particle_mod_data->data.lifetime_seed.max_base_time);
            break;
        case ui_particle_mod_type_location_orbit:
            break;
        case ui_particle_mod_type_location_seed:
            ui_data_np_save_float(s, 2, "MinBaseLocationX", particle_mod_data->data.location_seed.min_base_location.value[0]);
            ui_data_np_save_float(s, 2, "MinBaseLocationY", particle_mod_data->data.location_seed.min_base_location.value[1]);
            ui_data_np_save_float(s, 2, "MinBaseLocationZ", particle_mod_data->data.location_seed.min_base_location.value[2]);
            ui_data_np_save_float(s, 2, "MaxBaseLocationX", particle_mod_data->data.location_seed.max_base_location.value[0]);
            ui_data_np_save_float(s, 2, "MaxBaseLocationY", particle_mod_data->data.location_seed.max_base_location.value[1]);
            ui_data_np_save_float(s, 2, "MaxBaseLocationZ", particle_mod_data->data.location_seed.max_base_location.value[2]);
            break;
        case ui_particle_mod_type_rotation2d_seed:
            ui_data_np_save_float(s, 2, "MinSpinRate", particle_mod_data->data.rotation2d_seed.min_spin_rate);
            ui_data_np_save_float(s, 2, "MaxSpinRate", particle_mod_data->data.rotation2d_seed.max_spin_rate);
            ui_data_np_save_float(s, 2, "MinInitSpin", particle_mod_data->data.rotation2d_seed.min_init_spin);
            ui_data_np_save_float(s, 2, "MaxInitSpin", particle_mod_data->data.rotation2d_seed.max_init_spin);
            break;
        case ui_particle_mod_type_size_curved_uniform:
            break;
        case ui_particle_mod_type_size_curved:
            break;
        case ui_particle_mod_type_size_uniform_over_life:
            ui_data_np_save_float(s, 2, "MinBaseSize", particle_mod_data->data.size_uniform_over_life.min_base_size);
            ui_data_np_save_float(s, 2, "MaxBaseSize", particle_mod_data->data.size_uniform_over_life.max_base_size);
            ui_data_np_save_float(s, 2, "FadeBeginTime", particle_mod_data->data.size_uniform_over_life.fade_begin_time);
            break;
        case ui_particle_mod_type_size_over_life:
            break;
        case ui_particle_mod_type_size_seed:
            ui_data_np_save_float(s, 2, "MinBaseSizeX", particle_mod_data->data.size_seed.min_base_size.value[0]);
            ui_data_np_save_float(s, 2, "MinBaseSizeY", particle_mod_data->data.size_seed.min_base_size.value[1]);
            ui_data_np_save_float(s, 2, "MaxBaseSizeX", particle_mod_data->data.size_seed.max_base_size.value[0]);
            ui_data_np_save_float(s, 2, "MaxBaseSizeY", particle_mod_data->data.size_seed.max_base_size.value[1]);
            break;
        case ui_particle_mod_type_size_uniform_seed:
            ui_data_np_save_float(s, 2, "MinBaseSize", particle_mod_data->data.size_uniform_seed.min_base_size);
            ui_data_np_save_float(s, 2, "MaxBaseSize", particle_mod_data->data.size_uniform_seed.max_base_size);
            break;
        case ui_particle_mod_type_texcoord_flipbook_uv:
            ui_data_np_save_float(s, 2, "Framerate", particle_mod_data->data.texcoord_flipbook_uv.frame_rate);
            ui_data_np_save_bool(s, 2, "Loop", particle_mod_data->data.texcoord_flipbook_uv.loop);
            ui_data_np_save_int(s, 2, "StartTile", particle_mod_data->data.texcoord_flipbook_uv.start_tile_index);
            break;
        case ui_particle_mod_type_texcoord_scroll_anim:
            break;
        case ui_particle_mod_type_texcoord_tile_sub_tex:
            break;
        case ui_particle_mod_type_uber_circle_spawn:
            ui_data_np_save_float(s, 2, "AngleDelta", particle_mod_data->data.uber_circle_spawn.angle_delta);
            ui_data_np_save_float(s, 2, "CircleRadius", particle_mod_data->data.uber_circle_spawn.circle_radius);
            ui_data_np_save_float(s, 2, "MinRadialAccel", particle_mod_data->data.uber_circle_spawn.min_radial_accel);
            ui_data_np_save_float(s, 2, "MaxRadialAccel", particle_mod_data->data.uber_circle_spawn.max_radial_accel);
            ui_data_np_save_float(s, 2, "MinTangentAccel", particle_mod_data->data.uber_circle_spawn.min_tangent_accel);
            ui_data_np_save_float(s, 2, "MaxTangentAccel", particle_mod_data->data.uber_circle_spawn.max_tangent_accel);
            ui_data_np_save_float(s, 2, "MinRadialVelocity", particle_mod_data->data.uber_circle_spawn.min_radial_velocity);
            ui_data_np_save_float(s, 2, "MaxRadialVelocity", particle_mod_data->data.uber_circle_spawn.max_radial_velocity);
            ui_data_np_save_float(s, 2, "MinArcAngle", particle_mod_data->data.uber_circle_spawn.min_arc_angle);
            ui_data_np_save_float(s, 2, "MaxArcAngle", particle_mod_data->data.uber_circle_spawn.max_arc_angle);
            ui_data_np_save_float(s, 2, "DistributeDelta", particle_mod_data->data.uber_circle_spawn.distribute_delta);
            ui_data_np_save_float(s, 2, "InitialAngle", particle_mod_data->data.uber_circle_spawn.initial_angle);
            break;
        case ui_particle_mod_type_uber_ellipse_spawn:
            break;
        case ui_particle_mod_type_velocity_attract:
            break;
        case ui_particle_mod_type_velocity_seed:
            ui_data_np_save_float(s, 2, "MinBaseVelocityX", particle_mod_data->data.velocity_seed.min_base_velocity.value[0]);
            ui_data_np_save_float(s, 2, "MinBaseVelocityY", particle_mod_data->data.velocity_seed.min_base_velocity.value[1]);
            ui_data_np_save_float(s, 2, "MinBaseVelocityZ", particle_mod_data->data.velocity_seed.min_base_velocity.value[2]);
            ui_data_np_save_float(s, 2, "MaxBaseVelocityX", particle_mod_data->data.velocity_seed.max_base_velocity.value[0]);
            ui_data_np_save_float(s, 2, "MaxBaseVelocityY", particle_mod_data->data.velocity_seed.max_base_velocity.value[1]);
            ui_data_np_save_float(s, 2, "MaxBaseVelocityZ", particle_mod_data->data.velocity_seed.max_base_velocity.value[2]);
            ui_data_np_save_float(s, 2, "MinMultiplier", particle_mod_data->data.velocity_seed.min_multiplier);
            ui_data_np_save_float(s, 2, "MaxMultiplier", particle_mod_data->data.velocity_seed.max_multiplier);
            break;
        case ui_particle_mod_type_velocity_threshold_stop:
            break;
        }

        stream_printf(s, "    </MOD>\n");
    }

    if (emitter_data->tiling_u != 1.0f) ui_data_np_save_float(s, 1, "TilingU", emitter_data->tiling_u);
    if (emitter_data->tiling_v != 1.0f) ui_data_np_save_float(s, 1, "TilingV", emitter_data->tiling_v);
    if (emitter_data->blend_mode != 0) ui_data_np_save_int(s, 1, "BlendMode", emitter_data->blend_mode);
    if (emitter_data->atlas_x != 0.0f) ui_data_np_save_float(s, 1, "TexAtlasBiasX", emitter_data->atlas_x);
    if (emitter_data->atlas_y != 0.0f) ui_data_np_save_float(s, 1, "TexAtlasBiasY", emitter_data->atlas_y);
    if (emitter_data->atlas_w != 0.0f) ui_data_np_save_float(s, 1, "TexAtlasSizeX", emitter_data->atlas_w);
    if (emitter_data->atlas_h != 0.0f) ui_data_np_save_float(s, 1, "TexAtlasSizeY", emitter_data->atlas_h);
    if (emitter_data->origin != UI_PARTICLE_ORIGIN_CENTER) ui_data_np_save_int(s, 1, "Origin", emitter_data->origin);
    if (emitter_data->texture[0]) ui_data_np_save_str(s, 1, "BaseTexel", emitter_data->texture);

    stream_printf(s, "</NPParticleSpriteSRC>\n");

    return 0;
}

static int ui_data_np_save_particle_i(write_stream_t s, ui_data_particle_t particle, error_monitor_t em) {
    struct ui_data_particle_emitter_it emitter_it;
    ui_data_particle_emitter_t emitter;
    int rv = 0;

    stream_printf(s, "<?xml version=\"1.0\" encoding=\"UTF8\" ?>\n");

    ui_data_particle_emitters(&emitter_it, particle);

    while((emitter = ui_data_particle_emitter_it_next(&emitter_it))) {
        if (ui_data_np_save_emitter_i(s, emitter, em) != 0) {
            rv = -1;
        }
    }

    return rv;
}

int ui_data_np_save_particle(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em) {
    FILE * fp = NULL;
    int rv = -1;
    struct write_stream_file fs;

    fp = ui_data_np_save_open_file(root, src, em);
    if (fp == NULL) goto COMPLETE;

    write_stream_file_init(&fs, fp, em);
    rv = ui_data_np_save_particle_i((write_stream_t)&fs, ui_data_src_product(src), em);

COMPLETE:
    if (fp) file_stream_close(fp, em);
    return rv;
}

