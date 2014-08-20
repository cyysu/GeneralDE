#include "cpe/utils/stream_mem.h"
#include "cpe/utils/error.h"
#include "cpe/cfg/cfg_manage.h"
#include "cpe/utils/tests-env/with_em.hpp"
#include "gd/app/tests-env/with_app.hpp"
#include "ui/sprite/tests-env/with_world.hpp"
#include "ui/sprite_fsm/tests-env/with_fsm.hpp"
#include "ui/sprite_anim/ui_sprite_anim_module.h"
#include "ui/sprite_anim/tests-env/with_anim.hpp"

namespace ui { namespace sprite { namespace testenv {

with_anim::with_anim() : m_module(NULL) {
}

void with_anim::SetUp() {
    Base::SetUp();

    m_module = ui_sprite_anim_module_create(
        envOf<gd::app::testenv::with_app>().t_app(),
        envOf<with_world>().t_s_repo(),
        envOf<with_fsm>().t_s_fsm_repo(),
        t_allocrator(),
        NULL,
        envOf<utils::testenv::with_em>().t_em());

    ASSERT_TRUE(m_module != NULL);
}

void with_anim::TearDown() {
    ui_sprite_anim_module_free(m_module);
    m_module = NULL;

    Base::TearDown();
}

}}}
