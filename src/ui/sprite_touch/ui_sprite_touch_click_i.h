#ifndef UI_SPRITE_TOUCH_CLICK_I_H
#define UI_SPRITE_TOUCH_CLICK_I_H
#include "ui/sprite_touch/ui_sprite_touch_click.h"
#include "ui_sprite_touch_responser_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_touch_click {
    struct ui_sprite_touch_responser m_responser;
    char * m_on_click_down;
    char * m_on_click_up;
};

int ui_sprite_touch_click_regist(ui_sprite_touch_mgr_t module);
void ui_sprite_touch_click_unregist(ui_sprite_touch_mgr_t module);

#ifdef __cplusplus
}
#endif

#endif

