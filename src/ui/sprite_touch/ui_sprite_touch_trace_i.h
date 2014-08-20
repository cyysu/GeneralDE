#ifndef UI_SPRITE_TOUCH_TRACE_I_H
#define UI_SPRITE_TOUCH_TRACE_I_H
#include "ui/sprite_touch/ui_sprite_touch_trace.h"
#include "ui_sprite_touch_mgr_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_touch_trace {
    int32_t m_id;
    TAILQ_ENTRY(ui_sprite_touch_trace) m_next_for_mgr;
    ui_sprite_touch_responser_binding_list_t m_bindings;
};

ui_sprite_touch_trace_t
ui_sprite_touch_trace_create(ui_sprite_touch_mgr_t mgr, int32_t id);

void ui_sprite_touch_trace_free(ui_sprite_touch_mgr_t mgr, ui_sprite_touch_trace_t trace);
void ui_sprite_touch_trace_free_all(ui_sprite_touch_mgr_t mgr);

#ifdef __cplusplus
}
#endif

#endif
