#ifndef UI_MODEL_DATA_ACTION_H
#define UI_MODEL_DATA_ACTION_H
#include "protocol/ui/model/ui_action.h"
#include "ui_model_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_data_actor_it {
    ui_data_actor_t (*next)(struct ui_data_actor_it * it);
    char m_data[64];
};

struct ui_data_actor_layer_it {
    ui_data_actor_layer_t (*next)(struct ui_data_actor_layer_it * it);
    char m_data[64];
};

struct ui_data_actor_frame_it {
    ui_data_actor_frame_t (*next)(struct ui_data_actor_frame_it * it);
    char m_data[64];
};

/*action*/
ui_data_action_t ui_data_action_create(ui_data_mgr_t mgr, ui_data_src_t src);
void ui_data_action_free(ui_data_action_t action);
void ui_data_action_actors(ui_data_actor_it_t it, ui_data_action_t action);
int ui_data_action_update_refs(ui_data_action_t action);
void ui_data_action_use_srcs(ui_data_src_it_t it, ui_data_action_t action);
void ui_data_action_use_refs(ui_data_src_ref_it_t it, ui_data_action_t action);

/*actor*/
ui_data_actor_t ui_data_actor_create(ui_data_action_t action);
void ui_data_actor_free(ui_data_actor_t actor);
UI_ACTOR * ui_data_actor_data(ui_data_actor_t actor);
LPDRMETA ui_data_actor_meta(ui_data_mgr_t mgr);
void ui_data_actor_layers(ui_data_actor_layer_it_t it, ui_data_actor_t actor);

#define ui_data_actor_it_next(it) ((it)->next ? (it)->next(it) : NULL)

/*actor_layer*/
ui_data_actor_layer_t ui_data_actor_layer_create(ui_data_actor_t actor);
void ui_data_actor_layer_free(ui_data_actor_layer_t actor_layer);
void ui_data_actor_layer_frames(ui_data_actor_frame_it_t it, ui_data_actor_layer_t actor_layer);
UI_ACTOR_LAYER * ui_data_actor_layer_data(ui_data_actor_layer_t actor_layer);
LPDRMETA ui_data_actor_layer_meta(ui_data_mgr_t data_mgr);

#define ui_data_actor_layer_it_next(it) ((it)->next ? (it)->next(it) : NULL)

/*actor_frame*/
ui_data_actor_frame_t ui_data_actor_frame_create(ui_data_actor_layer_t actor_layer);
void ui_data_actor_frame_free(ui_data_actor_frame_t actor_frame);
UI_ACTOR_FRAME * ui_data_actor_frame_data(ui_data_actor_frame_t actor_frame);
LPDRMETA ui_data_actor_frame_meta(ui_data_mgr_t data_mgr);

#define ui_data_actor_frame_it_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif
