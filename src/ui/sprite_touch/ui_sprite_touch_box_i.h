#ifndef UI_SPRITE_TOUCH_BOX_I_H
#define UI_SPRITE_TOUCH_BOX_I_H
#include "ui/sprite_anim/ui_sprite_anim_types.h"
#include "ui/sprite_touch/ui_sprite_touch_box.h"
#include "ui_sprite_touch_touchable_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_touch_box {
    ui_sprite_touch_touchable_t m_touchable;
    UI_SPRITE_2D_PAIR m_lt;
    UI_SPRITE_2D_PAIR m_rb;
    uint32_t m_box_id;

    TAILQ_ENTRY(ui_sprite_touch_box) m_next_for_touchable;
};

#ifdef __cplusplus
}
#endif

#endif
