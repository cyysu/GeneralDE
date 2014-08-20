#include "CameraTest.hpp"
#include "../ui_sprite_anim_camera_i.h"

class CameraTraceXAdjTest : public CameraTest {
public:
    virtual void SetUp() {
        CameraTest::SetUp();

        t_s_camera_setup(
            "screen-size: { x: 960, y: 640 }\n"
            "trace-by-x:\n"
            "  screen: { x: 0.4, y: 0.3 }\n"
            "  world-a: { x: 0, y: 640 }\n"
            "  world-b: { x: 1920, y: 640 }\n"
            );

        t_s_camera_set_with_trace_x(500, 1.0f);

        ASSERT_CAMERA_RECT_EQ("(500,448)-(1460,1088)");
    }
};

TEST_F(CameraTraceXAdjTest, no_lock_basic) {
    UI_SPRITE_2D_PAIR camera_pos;
    float camera_scale = 1.0f;

    ui_sprite_anim_camera_adj_camera_in_limit(t_s_camera(), &camera_pos, &camera_scale);
}
