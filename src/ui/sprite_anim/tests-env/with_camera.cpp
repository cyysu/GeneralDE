#include "cpe/cfg/tests-env/with_cfg.hpp"
#include "ui/sprite/tests-env/with_world.hpp"
#include "ui/sprite_2d/tests-env/with_2d.hpp"
#include "ui/sprite_anim/tests-env/with_camera.hpp"
#include "ui/sprite_anim/tests-env/with_anim.hpp"
#include "../ui_sprite_anim_camera_i.h"

namespace ui { namespace sprite { namespace testenv {

with_camera::with_camera() : m_camera(NULL) {
}

void with_camera::SetUp() {
    Base::SetUp();

    m_camera = ui_sprite_anim_camera_create(
        envOf<with_anim>().t_s_anim_module(),
        envOf<with_world>().t_s_world());

    ASSERT_TRUE(m_camera != NULL);
}

void with_camera::TearDown() {
    ui_sprite_anim_camera_free(m_camera);
    m_camera = NULL;

    Base::TearDown();
}

void with_camera::t_s_camera_setup(const char * def) {
    t_s_camera_setup(envOf<cpe::cfg::testenv::with_cfg>().t_cfg_parse(def));
}

void with_camera::t_s_camera_set_with_trace_x(float x, float scale) {
    UI_SPRITE_2D_PAIR pos;
    pos.x = x;
    pos.y = ui_sprite_anim_camera_trace_x2y(m_camera, x, scale);
    ui_sprite_anim_camera_set_pos_and_scale(m_camera, pos, scale);
}

UI_SPRITE_2D_RECT
with_camera::t_s_camera_rect(void) {
    UI_SPRITE_2D_RECT r;
    ui_sprite_anim_camera_rect(m_camera, &r);
    return r;
}

const char * with_camera::t_s_camera_rect_str(void) {
    UI_SPRITE_2D_RECT rect = t_s_camera_rect();
    return envOf<with_2d>().t_s_2d_rect_dump(rect);
}

void with_camera::t_s_camera_setup(cfg_t cfg) {
    if (cfg_t screen_size_cfg = cfg_find_cfg(cfg, "screen-size")) {
        UI_SPRITE_2D_PAIR screen_size;
        screen_size.x = cfg_get_float(screen_size_cfg, "x", 0.0f);
        screen_size.y = cfg_get_float(screen_size_cfg, "y", 0.0f);
        ui_sprite_anim_camera_set_screen_size(m_camera, screen_size);
    }

    if (cfg_t limit_rect_cfg = cfg_find_cfg(cfg, "world-limit")) {
        UI_SPRITE_2D_RECT limit_rect;
        limit_rect.lt.x = cfg_get_float(limit_rect_cfg, "lt.x", 0.0f);
        limit_rect.lt.y = cfg_get_float(limit_rect_cfg, "lt.y", 0.0f);
        limit_rect.rb.x = cfg_get_float(limit_rect_cfg, "rb.x", 0.0f);
        limit_rect.rb.y = cfg_get_float(limit_rect_cfg, "rb.y", 0.0f);

        ASSERT_EQ(0, ui_sprite_anim_camera_set_limit(m_camera, limit_rect.lt, limit_rect.rb))
            << "set camera limit (" << limit_rect.lt.x << "," << limit_rect.lt.y << ")-("
            << limit_rect.rb.x << "," << limit_rect.rb.y << ") fail";
    }

    if (cfg_t camera_cfg = cfg_find_cfg(cfg, "camera")) {
        float scale = cfg_get_float(camera_cfg, "scale", 0.0f);
        UI_SPRITE_2D_PAIR pos;
        pos.x = cfg_get_float(camera_cfg, "pos.x", 0.0f);
        pos.y = cfg_get_float(camera_cfg, "pos.y", 0.0f);

        ui_sprite_anim_camera_set_pos_and_scale(m_camera, pos, scale);
    }

    if (cfg_t trace_x_cfg = cfg_find_cfg(cfg, "trace-by-x")) {
        UI_SPRITE_2D_PAIR screen_pos;
        UI_SPRITE_2D_PAIR world_pos_a;
        UI_SPRITE_2D_PAIR world_pos_b;

        screen_pos.x = cfg_get_float(trace_x_cfg, "screen.x", 0.0f);
        screen_pos.y = cfg_get_float(trace_x_cfg, "screen.y", 0.0f);

        world_pos_a.x = cfg_get_float(trace_x_cfg, "world-a.x", 0.0f);
        world_pos_a.y = cfg_get_float(trace_x_cfg, "world-a.y", 0.0f);

        world_pos_b.x = cfg_get_float(trace_x_cfg, "world-b.x", 0.0f);
        world_pos_b.y = cfg_get_float(trace_x_cfg, "world-b.y", 0.0f);

        ASSERT_EQ(
            0, 
            ui_sprite_anim_camera_set_trace(
                m_camera, ui_sprite_anim_camera_trace_by_x,
                screen_pos, world_pos_a, world_pos_b))
            << "set camera trace-by-x:"
            << ", screen-pos=(" << screen_pos.x << "," << screen_pos.y << ")"
            << ", world-pos-a=(" << world_pos_a.x << "," << world_pos_a.y << ")"
            << ", world-pos-b=(" << world_pos_b.x << "," << world_pos_b.y << ")"
            << "  fail!";
    }
}

}}}
