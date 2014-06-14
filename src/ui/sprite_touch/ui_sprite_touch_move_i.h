#ifndef UI_SPRITE_TOUCH_MOVE_I_H
#define UI_SPRITE_TOUCH_MOVE_I_H
#include "ui/sprite_touch/ui_sprite_touch_move.h"
#include "ui_sprite_touch_responser_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_touch_move {
    struct ui_sprite_touch_responser m_responser;
    char * m_on_begin;
    char * m_on_move;
    char * m_on_end;
    char * m_on_cancel;
    float m_stick_duration;
    UI_SPRITE_TOUCH_MOVE_STATE m_state; /*action data*/
};

int ui_sprite_touch_move_regist(ui_sprite_touch_mgr_t module);
void ui_sprite_touch_move_unregist(ui_sprite_touch_mgr_t module);

#ifdef __cplusplus
}
#endif

#endif

