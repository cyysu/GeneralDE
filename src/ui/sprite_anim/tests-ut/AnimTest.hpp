#ifndef UI_SPRITE_ANIM_UT_TEST_H
#define UI_SPRITE_ANIM_UT_TEST_H
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/tests-env/with_em.hpp"
#include "cpe/cfg/tests-env/with_cfg.hpp"
#include "gd/app/tests-env/with_app.hpp"
#include "ui/sprite/tests-env/with_world.hpp"
#include "ui/sprite/tests-env/with_entity.hpp"
#include "ui/sprite_fsm/tests-env/with_fsm.hpp"
#include "ui/sprite_2d/tests-env/with_2d.hpp"
#include "ui/sprite_anim/tests-env/with_anim.hpp"

typedef LOKI_TYPELIST_8(
    utils::testenv::with_em
    , cpe::cfg::testenv::with_cfg
    , gd::app::testenv::with_app
    , ui::sprite::testenv::with_world
    , ui::sprite::testenv::with_entity
    , ui::sprite::testenv::with_fsm
    , ui::sprite::testenv::with_2d
    , ui::sprite::testenv::with_anim) AnimTestBase;

class AnimTest : public testenv::fixture<AnimTestBase> {
public:
    AnimTest();

    virtual void SetUp();
};

#endif
