#ifndef UI_SPRITE_TOUCH_RESPONSER_I_H
#define UI_SPRITE_TOUCH_RESPONSER_I_H
#include "ui_sprite_touch_trace_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_sprite_touch_responser_binding {
    ui_sprite_touch_trace_t m_trace;
    UI_SPRITE_2D_PAIR m_start_screen_pt;
    UI_SPRITE_2D_PAIR m_start_world_pt;
    UI_SPRITE_2D_PAIR m_pre_screen_pt;
    UI_SPRITE_2D_PAIR m_pre_world_pt;
    UI_SPRITE_2D_PAIR m_cur_screen_pt;
    UI_SPRITE_2D_PAIR m_cur_world_pt;
    TAILQ_ENTRY(ui_sprite_touch_responser) m_next;
} * ui_sprite_touch_responser_binding_t;

struct ui_sprite_touch_responser {
    ui_sprite_touch_touchable_t m_touchable;
    uint8_t m_finger_count;
    uint8_t m_is_capture;
    uint8_t m_is_grab;
    uint8_t m_is_start;
    uint8_t m_threshold;
    TAILQ_ENTRY(ui_sprite_touch_responser) m_next_for_touchable;
    TAILQ_ENTRY(ui_sprite_touch_responser) m_next_for_mgr;

    uint8_t m_binding_count;
    struct ui_sprite_touch_responser_binding m_bindings[UI_SPRITE_TOUCH_MAX_FINGER_COUNT];

    void (*m_on_begin)(void * responser);
    void (*m_on_move)(void * responser);
    void (*m_on_end)(void * responser);
    void (*m_on_cancel)(void * responser);
};

void ui_sprite_touch_responser_free(ui_sprite_touch_responser_t responser);
int ui_sprite_touch_responser_set_finger_count(ui_sprite_touch_responser_t responser, uint8_t finger_count);
int ui_sprite_touch_responser_set_is_capture(ui_sprite_touch_responser_t responser, uint8_t is_capture);
int ui_sprite_touch_responser_set_is_grab(ui_sprite_touch_responser_t responser, uint8_t is_grab);

void ui_sprite_touch_responser_cancel(ui_sprite_touch_responser_t responser);

int ui_sprite_touch_responser_init(
    ui_sprite_touch_responser_t responser,
    ui_sprite_entity_t entity,
    ui_sprite_touch_touchable_t touchable);

int ui_sprite_touch_responser_copy(ui_sprite_touch_responser_t to, ui_sprite_touch_responser_t from);
void ui_sprite_touch_responser_fini(ui_sprite_touch_responser_t responser);
int ui_sprite_touch_responser_enter(ui_sprite_touch_responser_t responser);
void ui_sprite_touch_responser_exit(ui_sprite_touch_responser_t responser);

uint8_t ui_sprite_touch_responser_bind_tracer(ui_sprite_touch_responser_t responser, ui_sprite_touch_trace_t trace);
void ui_sprite_touch_responser_unbind_tracer(ui_sprite_touch_responser_t responser, ui_sprite_touch_trace_t trace);
int8_t ui_sprite_touch_responser_binding_find(ui_sprite_touch_responser_t responser, ui_sprite_touch_trace_t trace);

void ui_sprite_touch_responser_on_begin(ui_sprite_touch_responser_t responser, uint8_t binding_pos);
void ui_sprite_touch_responser_on_move(ui_sprite_touch_responser_t responser, uint8_t binding_pos);
void ui_sprite_touch_responser_on_end(ui_sprite_touch_responser_t responser, uint8_t binding_pos);

#ifdef __cplusplus
}
#endif

#endif
