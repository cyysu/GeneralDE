#include "ui/sprite_anim/ui_sprite_anim_camera_contain.h"
#include "CameraTest.hpp"

class CameraTraceXActionContainTest : public CameraTest {
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

    void apply_action(const char * evt) {
        ui_sprite_anim_camera_contain_t contain =
            ui_sprite_anim_camera_contain_create(t_s_fsm_test_state(), "");
        ASSERT_TRUE(contain);

        t_s_fsm_action_endless(contain);
        t_s_fsm_action_enter(contain);
        t_s_entity_send_event(evt);
        t_app_tick();

        t_s_fsm_action_update(contain, 0);
    }
};

TEST_F(CameraTraceXActionContainTest, single_in_screen) {
    t_s_entity_create(
        "a",
        "Transform:\n"
        "  pos: { x: 960, y: 640 }\n"
        "  rect:\n"
        "    lt: { x: -5, y: -5 }\n"
        "    rb: { x: 5, y: 5 }\n"
        );
    t_s_group_create("group_1", "a");

    t_em_set_print();

    apply_action(
        "ui_sprite_evt_anim_camera_contain_group:"
        "    group_name=group_1,"
        "    screen_lt.x=0.1,"
        "    screen_lt.y=0.1,"
        "    screen_rb.x=0.9,"
        "    screen_rb.y=0.9,"
        );

    ASSERT_CAMERA_RECT_EQ("(500,448)-(1460,1088)");
}

TEST_F(CameraTraceXActionContainTest, single_rb_scale) {
    t_s_entity_create(
        "a",
        "Transform:\n"
        "  pos: { x: 1460, y: 1088 }\n"
        "  rect:\n"
        "    lt: { x: -5, y: -5 }\n"
        "    rb: { x: 5, y: 5 }\n"
        );
    t_s_group_create("group_1", "a");

    apply_action(
        "ui_sprite_evt_anim_camera_contain_group:"
        "    group_name=group_1,"
        "    screen_lt.x=0.1,"
        "    screen_lt.y=0.1,"
        "    screen_rb.x=0.9,"
        "    screen_rb.y=0.9,"
        );

    //ASSERT_CAMERA_RECT_EQ("(500,448)-(1465,1093)");
}

