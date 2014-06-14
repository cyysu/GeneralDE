#ifndef UI_SPRITE_FSM_TYPES_H
#define UI_SPRITE_FSM_TYPES_H
#include "ui/sprite/ui_sprite_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ui_sprite_fsm_action_state {
    ui_sprite_fsm_action_state_deactive = 1,
    ui_sprite_fsm_action_state_waiting,
    ui_sprite_fsm_action_state_runing,
    ui_sprite_fsm_action_state_done,
} ui_sprite_fsm_action_state_t;

typedef enum ui_sprite_fsm_action_life_circle {
    ui_sprite_fsm_action_life_circle_passive = 1,
    ui_sprite_fsm_action_life_circle_working,
    ui_sprite_fsm_action_life_circle_endless,
    ui_sprite_fsm_action_life_circle_duration,
} ui_sprite_fsm_action_life_circle_t;

typedef struct ui_sprite_fsm_module * ui_sprite_fsm_module_t;
typedef struct ui_sprite_fsm_ins * ui_sprite_fsm_ins_t;
typedef struct ui_sprite_fsm_state * ui_sprite_fsm_state_t;
typedef struct ui_sprite_fsm_action * ui_sprite_fsm_action_t;
typedef struct ui_sprite_fsm_action_meta * ui_sprite_fsm_action_meta_t;
typedef struct ui_sprite_fsm_transition * ui_sprite_fsm_transition_t;

#ifdef __cplusplus
}
#endif

#endif
