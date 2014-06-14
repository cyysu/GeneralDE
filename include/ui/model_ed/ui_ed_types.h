#ifndef UI_MODEL_ED_TYPES_H
#define UI_MODEL_ED_TYPES_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "../model/ui_model_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ui_ed_obj_type {
    ui_ed_obj_src = 1
    , ui_ed_obj_module = 2
    , ui_ed_obj_img_block = 3
    , ui_ed_obj_frame = 4
    , ui_ed_obj_frame_img = 5
    , ui_ed_obj_actor = 6
    , ui_ed_obj_actor_layer = 7
    , ui_ed_obj_actor_frame = 8
    , ui_ed_obj_particle_emitter = 9
    , ui_ed_obj_particle_mod = 10
} ui_ed_obj_type_t;
#define UI_ED_OBJ_TYPE_MIN 1
#define UI_ED_OBJ_TYPE_MAX 11

typedef enum ui_ed_rel_type {
    ui_ed_rel_type_use_img = 1
    , ui_ed_rel_type_use_actor = 2
} ui_ed_rel_type_t;
#define UI_ED_REL_TYPE_MIN 1
#define UI_ED_REL_TYPE_MAX 3

typedef enum ui_ed_src_state {
    ui_ed_src_state_normal = 0
    , ui_ed_src_state_new = 1
    , ui_ed_src_state_removed = 2
    , ui_ed_src_state_changed = 3
} ui_ed_src_state_t;

typedef struct ui_ed_mgr * ui_ed_mgr_t;

typedef struct ui_ed_search * ui_ed_search_t;

typedef struct ui_ed_src * ui_ed_src_t;

typedef struct ui_ed_obj_meta * ui_ed_obj_meta_t;
typedef struct ui_ed_obj * ui_ed_obj_t;
typedef struct ui_ed_obj_it * ui_ed_obj_it_t;

typedef struct ui_ed_rel_meta * ui_ed_rel_meta_t;
typedef struct ui_ed_rel * ui_ed_rel_t;

typedef struct ui_ed_op * ui_ed_op_t;

typedef struct ui_ed_change * ui_ed_change_t;

#ifdef __cplusplus
}
#endif

#endif
