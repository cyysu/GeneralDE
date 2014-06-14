#ifndef UI_DATA_ACTION_INTERNAL_H
#define UI_DATA_ACTION_INTERNAL_H
#include "ui/model/ui_data_action.h"
#include "ui_model_internal_types.h"
#include "ui_data_src_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_data_action_use * ui_data_action_use_t;

typedef TAILQ_HEAD(ui_data_action_use_list, ui_data_action_use) ui_data_action_use_list_t;
typedef TAILQ_HEAD(ui_data_actor_list, ui_data_actor) ui_data_actor_list_t;
typedef TAILQ_HEAD(ui_data_actor_layer_list, ui_data_actor_layer) ui_data_actor_layer_list_t;
typedef TAILQ_HEAD(ui_data_actor_frame_list, ui_data_actor_frame) ui_data_actor_frame_list_t;

struct ui_data_action {
    ui_data_mgr_t m_mgr;
    ui_data_src_t m_src;
    ui_data_actor_list_t m_actors;
    ui_data_action_use_list_t m_uses;
};

struct ui_data_action_use {
    ui_data_src_ref_t m_use;
    ui_data_actor_frame_list_t m_user_frames;
    TAILQ_ENTRY(ui_data_action_use) m_next_for_action;
};

struct ui_data_actor {
    ui_data_action_t m_action;
    struct cpe_hash_entry m_hh_for_mgr;
    TAILQ_ENTRY(ui_data_actor) m_next_for_action;
    ui_data_actor_layer_list_t m_layers;
    UI_ACTOR m_data;
};

struct ui_data_actor_layer {
    ui_data_actor_t m_actor;
    TAILQ_ENTRY(ui_data_actor_layer) m_next_for_actor;
    ui_data_actor_frame_list_t m_frames;
    UI_ACTOR_LAYER m_data;
};

struct ui_data_actor_frame {
    ui_data_actor_layer_t m_layer;
    TAILQ_ENTRY(ui_data_actor_frame) m_next_for_layer;
    ui_data_action_use_t m_use;
    TAILQ_ENTRY(ui_data_actor_frame) m_next_for_use;
    UI_ACTOR_FRAME m_data;
};

void ui_data_action_use_remove_frame(ui_data_actor_frame_t actor_frame);
int ui_data_action_use_add_frame(ui_data_actor_frame_t actor_frame);

#ifdef __cplusplus
}
#endif

#endif
