#ifndef UI_SPRITE_ANIM_TESTENV_WITH_CAMERA_H
#define UI_SPRITE_ANIM_TESTENV_WITH_CAMERA_H
#include "cpe/utils/tests-env/test-env.hpp"
#include "../ui_sprite_anim_camera.h"

namespace ui { namespace sprite { namespace testenv {

class with_camera : public ::testenv::env<> {
public:
    with_camera();

    void SetUp();
    void TearDown();

    ui_sprite_anim_camera_t t_s_camera(void) { return m_camera; }

    void t_s_camera_setup(const char * def);
    void t_s_camera_setup(cfg_t cfg);

    void t_s_camera_set_with_trace_x(float x, float scale);

    const char * t_s_camera_rect_str(void);
    UI_SPRITE_2D_RECT t_s_camera_rect(void);

private:
    ui_sprite_anim_camera_t m_camera;
};

#define ASSERT_CAMERA_RECT_EQ(__rect)                                   \
if (!t_s_2d_rect_eq(t_s_2d_rect_build(__rect), t_s_camera_rect())) {    \
    FAIL() << "camera rect mismatch:\n"                                 \
           << "    expect: " << (__rect) << "\n"                        \
           << "    but: " << t_s_camera_rect_str();                     \
}

}}}

#endif
