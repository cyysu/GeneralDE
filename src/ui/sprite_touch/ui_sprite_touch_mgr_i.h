#ifndef UI_SPRITE_TOUCH_MANAGER_I_H
#define UI_SPRITE_TOUCH_MANAGER_I_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/hash.h"
#include "ui/sprite_touch/ui_sprite_touch_mgr.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_sprite_touch_responser * ui_sprite_touch_responser_t;

typedef TAILQ_HEAD(ui_sprite_touch_touchable_list, ui_sprite_touch_touchable) ui_sprite_touch_touchable_list_t;
typedef TAILQ_HEAD(ui_sprite_touch_responser_list, ui_sprite_touch_responser) ui_sprite_touch_responser_list_t;
typedef TAILQ_HEAD(ui_sprite_touch_box_list, ui_sprite_touch_box) ui_sprite_touch_box_list_t;

typedef TAILQ_HEAD(ui_sprite_touch_trace_list, ui_sprite_touch_trace) ui_sprite_touch_trace_list_t;

struct ui_sprite_touch_mgr {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    ui_sprite_repository_t m_repo;
    ui_sprite_fsm_module_t m_fsm;
    int m_debug;
    uint8_t m_dft_threshold;

    LPDRMETA m_meta_move_state;
    LPDRMETA m_meta_scale_state;

    ui_sprite_touch_trace_list_t m_touch_traces;

    ui_sprite_touch_responser_list_t m_active_responsers;
    ui_sprite_touch_responser_list_t m_waiting_responsers;
};

ui_sprite_touch_responser_t ui_sprite_touch_mgr_find_grab_active_responser(ui_sprite_touch_mgr_t mgr);
tl_time_t ui_sprite_touch_mgr_cur_time(ui_sprite_touch_mgr_t mgr);

#ifdef __cplusplus
}
#endif

#endif
