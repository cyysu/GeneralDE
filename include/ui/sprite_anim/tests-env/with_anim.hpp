#ifndef UI_SPRITE_ANIM_TESTENV_WITH_ANIM_H
#define UI_SPRITE_ANIM_TESTENV_WITH_ANIM_H
#include "cpe/utils/tests-env/test-env.hpp"
#include "../ui_sprite_anim_types.h"

namespace ui { namespace sprite { namespace testenv {

class with_anim : public ::testenv::env<> {
public:
    with_anim();

    void SetUp();
    void TearDown();

    ui_sprite_anim_module_t t_s_anim_module(void) { return m_module; }

private:
    ui_sprite_anim_module_t m_module;
};

}}}

#endif
