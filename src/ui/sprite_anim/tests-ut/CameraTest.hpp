#ifndef UI_SPRITE_ANIM_UT_TEST_CAMERA_H
#define UI_SPRITE_ANIM_UT_TEST_CAMERA_H
#include "ui/sprite_anim/tests-env/with_camera.hpp"
#include "AnimTest.hpp"

typedef LOKI_TYPELIST_1(
    ui::sprite::testenv::with_camera) CameraTestBase;

class CameraTest : public testenv::fixture<CameraTestBase, AnimTest> {
public:
    CameraTest();
};

#endif
