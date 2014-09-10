#ifndef UI_SPRITE_2D_TESTENV_WITHWORLD_H
#define UI_SPRITE_2D_TESTENV_WITHWORLD_H
#include "cpe/utils/tests-env/test-env.hpp"
#include "../ui_sprite_2d_types.h"

namespace ui { namespace sprite { namespace testenv {

class with_2d : public ::testenv::env<> {
public:
    with_2d();

    void SetUp();
    void TearDown();

    ui_sprite_2d_module_t t_s_2d_module(void) { return m_module; }

    UI_SPRITE_2D_RECT t_s_2d_rect_build(const char * def);
    const char * t_s_2d_rect_dump(UI_SPRITE_2D_RECT const & rect);

    bool t_s_2d_rect_eq(UI_SPRITE_2D_RECT const & l, UI_SPRITE_2D_RECT const & r, float delta = 0.1f);
    bool t_s_2d_pair_eq(UI_SPRITE_2D_PAIR const & l, UI_SPRITE_2D_PAIR const & r, float delta = 0.1f);

private:
    static void build_2d_transform(void * ctx, void * component_data, cfg_t cfg);

    ui_sprite_2d_module_t m_module;
};

}}}

#endif
