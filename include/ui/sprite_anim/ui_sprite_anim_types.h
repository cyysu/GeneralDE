#ifndef UI_SPRITE_ANIM_TYPES_H
#define UI_SPRITE_ANIM_TYPES_H
#include "ui/sprite/ui_sprite_types.h"
#include "ui/sprite_2d/ui_sprite_2d_types.h"
#include "ui/sprite_fsm/ui_sprite_fsm_types.h"
#include "protocol/ui/sprite_anim/ui_sprite_anim_evt.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UI_SPRITE_INVALID_ANIM_ID ((uint32_t)0)
#define UI_SPRITE_INVALID_CAMERA_OP_ID ((uint32_t)0)

typedef struct ui_sprite_anim_module * ui_sprite_anim_module_t;
typedef struct ui_sprite_anim_backend * ui_sprite_anim_backend_t;

/*camera*/
typedef struct ui_sprite_anim_camera * ui_sprite_anim_camera_t;
typedef struct ui_sprite_anim_camera_restrict * ui_sprite_anim_camera_restrict_t;

/*animation*/
typedef struct ui_sprite_anim_sch * ui_sprite_anim_sch_t;
typedef struct ui_sprite_anim_def * ui_sprite_anim_def_t;
typedef struct ui_sprite_anim_def_it * ui_sprite_anim_def_it_t;
typedef struct ui_sprite_anim_group * ui_sprite_anim_group_t;
typedef struct ui_sprite_anim_template * ui_sprite_anim_template_t;
typedef struct ui_sprite_anim_template_binding * ui_sprite_anim_template_binding_t;

/*actions*/
typedef struct ui_sprite_anim_show_animation * ui_sprite_anim_show_animation_t;
typedef struct ui_sprite_anim_show_template * ui_sprite_anim_show_template_t;
typedef struct ui_sprite_anim_show_track * ui_sprite_anim_show_track_t;

typedef struct ui_sprite_anim_camera_touch * ui_sprite_anim_camera_touch_t;
typedef struct ui_sprite_anim_camera_move * ui_sprite_anim_camera_move_t;
typedef struct ui_sprite_anim_camera_scale * ui_sprite_anim_camera_scale_t;
typedef struct ui_sprite_anim_camera_in_area * ui_sprite_anim_camera_in_area_t;
typedef struct ui_sprite_anim_camera_shake * ui_sprite_anim_camera_shake_t;
typedef struct ui_sprite_anim_camera_wait_stop * ui_sprite_anim_camera_wait_stop_t;

#ifdef __cplusplus
}
#endif

#endif


