#ifndef UI_MODEL_TYPES_H
#define UI_MODEL_TYPES_H
#include "cpe/utils/error.h"
#include "cpe/pal/pal_types.h"
#include "cpe/dr/dr_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ui_data_src_type_dir = 1
    , ui_data_src_type_module = 2
    , ui_data_src_type_sprite = 3
    , ui_data_src_type_action = 4
    , ui_data_src_type_layout = 5
    , ui_data_src_type_particle = 6
    , ui_data_src_type_texture_png = 7
} ui_data_src_type_t;
#define UI_DATA_SRC_TYPE_MIN (1)
#define UI_DATA_SRC_TYPE_MAX (8)

typedef enum {
    ui_data_control_window = 1
    , ui_data_control_panel = 2
    , ui_data_control_picture = 3
    , ui_data_control_label = 4
    , ui_data_control_button = 5
    , ui_data_control_toggle = 6
    , ui_data_control_progress = 7
    , ui_data_control_picture_cond = 8
} ui_data_control_type_t;
#define UI_DATA_CONTROL_TYPE_MIN (1)
#define UI_DATA_CONTROL_TYPE_MAX (9)

typedef enum {
    ui_data_src_state_loaded = 1
    , ui_data_src_state_notload = 2
} ui_data_src_load_state_t;

/*for mgr*/
typedef struct ui_data_meta * ui_data_meta_t;
typedef struct ui_data_mgr * ui_data_mgr_t;

/*for src*/
typedef struct ui_data_src * ui_data_src_t;
typedef struct ui_data_src_ref * ui_data_src_ref_t;
typedef struct ui_data_src_it * ui_data_src_it_t;
typedef struct ui_data_src_ref_it * ui_data_src_ref_it_t;

typedef int (*product_load_fun_t)(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em);
typedef int (*product_save_fun_t)(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em);
typedef int (*product_remove_fun_t)(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em);

/*for module*/
typedef struct ui_data_module * ui_data_module_t;
typedef struct ui_data_img_block * ui_data_img_block_t;
typedef struct ui_data_img_block_it * ui_data_img_block_it_t;

/*for sprite*/
typedef struct ui_data_sprite * ui_data_sprite_t;
typedef struct ui_data_frame * ui_data_frame_t;
typedef struct ui_data_frame_img * ui_data_frame_img_t;
typedef struct ui_data_frame_it * ui_data_frame_it_t;
typedef struct ui_data_frame_img_it * ui_data_frame_img_it_t;

/*for action*/
typedef struct ui_data_action * ui_data_action_t;
typedef struct ui_data_actor * ui_data_actor_t;
typedef struct ui_data_actor_it * ui_data_actor_it_t;
typedef struct ui_data_actor_layer * ui_data_actor_layer_t;
typedef struct ui_data_actor_layer_it * ui_data_actor_layer_it_t;
typedef struct ui_data_actor_frame * ui_data_actor_frame_t;
typedef struct ui_data_actor_frame_it * ui_data_actor_frame_it_t;

/*for layout*/
typedef struct ui_data_layout * ui_data_layout_t;
typedef struct ui_data_control * ui_data_control_t;
typedef struct ui_data_control_it * ui_data_control_it_t;

/*for particle*/
typedef struct ui_data_particle * ui_data_particle_t;
typedef struct ui_data_particle_emitter * ui_data_particle_emitter_t;
typedef struct ui_data_particle_emitter_it * ui_data_particle_emitter_it_t;
typedef struct ui_data_particle_mod * ui_data_particle_mod_t;
typedef struct ui_data_particle_mod_it * ui_data_particle_mod_it_t;

#ifdef __cplusplus
}
#endif

#endif
