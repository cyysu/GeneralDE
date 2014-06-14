#include "ui/sprite_fsm/ui_sprite_fsm_component.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_fsm.h"
#include "ui/sprite_basic/ui_sprite_basic_send_event.h"
#include "ui/sprite_basic/ui_sprite_basic_noop.h"
#include "ui/sprite_basic/ui_sprite_basic_join_group.h"
#include "ui/sprite_basic/ui_sprite_basic_set_attrs.h"
#include "ui/sprite_basic/ui_sprite_basic_debug.h"
#include "ui/sprite_basic/ui_sprite_basic_gen_entities.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui/sprite_2d/ui_sprite_2d_move.h"
#include "ui/sprite_2d/ui_sprite_2d_scale.h"
#include "ui/sprite_2d/ui_sprite_2d_track_flip.h"
#include "ui/sprite_2d/ui_sprite_2d_track_angle.h"
#include "ui/sprite_2d/ui_sprite_2d_wait_switchback.h"
#include "ui/sprite_anim/ui_sprite_anim_camera.h"
#include "ui/sprite_anim/ui_sprite_anim_backend.h"
#include "ui/sprite_anim/ui_sprite_anim_sch.h"
#include "ui/sprite_anim/ui_sprite_anim_show_animation.h"
#include "ui/sprite_anim/ui_sprite_anim_show_template.h"
#include "ui/sprite_anim/ui_sprite_anim_show_track.h"
#include "ui/sprite_anim/ui_sprite_anim_camera_touch.h"
#include "ui/sprite_anim/ui_sprite_anim_camera_move.h"
#include "ui/sprite_anim/ui_sprite_anim_camera_scale.h"
#include "ui/sprite_anim/ui_sprite_anim_camera_in_area.h"
#include "ui/sprite_anim/ui_sprite_anim_camera_shake.h"
#include "ui/sprite_anim/ui_sprite_anim_camera_wait_stop.h"
#include "ui/sprite_touch/ui_sprite_touch_touchable.h"
#include "ui/sprite_touch/ui_sprite_touch_move.h"
#include "ui/sprite_touch/ui_sprite_touch_click.h"
#include "ui/sprite_ctrl/ui_sprite_ctrl_circle.h"
#include "ui_sprite_cfg_loader_i.h"

#define UI_SPRITE_CFG_INIT_RESOURCE(__name, __fun)                     \
    do {                                                                \
        extern ui_sprite_world_res_t ui_sprite_cfg_load_resource_ ## __fun(void * ctx, ui_sprite_world_t world, cfg_t cfg); \
            if (ui_sprite_cfg_loader_add_resource_loader(loader, __name, ui_sprite_cfg_load_resource_ ## __fun, loader) != 0) { \
                CPE_ERROR(loader->m_em, "%s: add default resource loader %s fail!", ui_sprite_cfg_loader_name(loader), __name); \
                return -1;                                              \
            }                                                           \
    } while(0)

#define UI_SPRITE_CFG_INIT_COMPONENT(__name, __fun)                     \
    do {                                                                \
        extern int ui_sprite_cfg_load_component_ ## __fun(void * ctx, ui_sprite_component_t component, cfg_t cfg); \
            if (ui_sprite_cfg_loader_add_comp_loader(loader, __name, ui_sprite_cfg_load_component_ ## __fun, loader) != 0) { \
                CPE_ERROR(loader->m_em, "%s: add default component loader %s fail!", ui_sprite_cfg_loader_name(loader), __name); \
                return -1;                                              \
            }                                                           \
    } while(0)

#define UI_SPRITE_CFG_INIT_ACTION(__name, __fun)                        \
    do {                                                                \
        extern ui_sprite_fsm_action_t ui_sprite_cfg_load_action_ ## __fun(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg); \
            if (ui_sprite_cfg_loader_add_action_loader(loader, __name, ui_sprite_cfg_load_action_ ## __fun, loader) != 0) { \
                CPE_ERROR(loader->m_em, "%s: add default action loader %s fail!", ui_sprite_cfg_loader_name(loader), __name); \
                return -1;                                              \
            }                                                           \
    } while(0)

int ui_sprite_cfg_loader_init_default_loaders(ui_sprite_cfg_loader_t loader) {
    /*resources*/
    UI_SPRITE_CFG_INIT_RESOURCE(UI_SPRITE_ANIM_CAMERA_TYPE_NAME, camera);
    UI_SPRITE_CFG_INIT_RESOURCE(UI_SPRITE_ANIM_BACKEND_NAME, anim_backend);

    /*components*/
    UI_SPRITE_CFG_INIT_COMPONENT(UI_SPRITE_FSM_COMPONENT_FSM_NAME, fsm);
    UI_SPRITE_CFG_INIT_COMPONENT(UI_SPRITE_2D_TRANSFORM_NAME, transform);
    UI_SPRITE_CFG_INIT_COMPONENT(UI_SPRITE_ANIM_SCH_NAME, animation);
    UI_SPRITE_CFG_INIT_COMPONENT(UI_SPRITE_TOUCH_TOUCHABLE_NAME, touchable);

    /*actions*/
    UI_SPRITE_CFG_INIT_ACTION(UI_SPRITE_FSM_ACTION_FSM_NAME, fsm);
    UI_SPRITE_CFG_INIT_ACTION(UI_SPRITE_BASIC_SEND_EVENT_NAME, send_event);
    UI_SPRITE_CFG_INIT_ACTION(UI_SPRITE_BASIC_NOOP_NAME, noop);
    UI_SPRITE_CFG_INIT_ACTION(UI_SPRITE_BASIC_JOIN_GROUP_NAME, join_group);
    UI_SPRITE_CFG_INIT_ACTION(UI_SPRITE_BASIC_GEN_ENTITIES_NAME, gen_entities);
    UI_SPRITE_CFG_INIT_ACTION(UI_SPRITE_BASIC_SET_ATTRS_NAME, set_attrs);
    UI_SPRITE_CFG_INIT_ACTION(UI_SPRITE_BASIC_DEBUG_NAME, debug);
    UI_SPRITE_CFG_INIT_ACTION(UI_SPRITE_2D_MOVE_NAME, 2d_move);
	UI_SPRITE_CFG_INIT_ACTION(UI_SPRITE_2D_SCALE_NAME, 2d_scale);
	UI_SPRITE_CFG_INIT_ACTION(UI_SPRITE_2D_TRACK_FLIP_NAME, 2d_track_flip);
	UI_SPRITE_CFG_INIT_ACTION(UI_SPRITE_2D_TRACK_ANGLE_NAME, 2d_track_angle);
	UI_SPRITE_CFG_INIT_ACTION(UI_SPRITE_2D_WAIT_SWITCHBACK_NAME, 2d_wait_switchback);
    UI_SPRITE_CFG_INIT_ACTION(UI_SPRITE_ANIM_SHOW_ANIMATION_TYPE_NAME, show_animation);
    UI_SPRITE_CFG_INIT_ACTION(UI_SPRITE_ANIM_SHOW_TEMPLATE_TYPE_NAME, show_template);
	UI_SPRITE_CFG_INIT_ACTION(UI_SPRITE_ANIM_SHOW_TRACK_NAME, show_track);
    UI_SPRITE_CFG_INIT_ACTION(UI_SPRITE_ANIM_CAMERA_TOUCH_NAME, camera_touch);
    UI_SPRITE_CFG_INIT_ACTION(UI_SPRITE_ANIM_CAMERA_MOVE_NAME, camera_move);
    UI_SPRITE_CFG_INIT_ACTION(UI_SPRITE_ANIM_CAMERA_SCALE_NAME, camera_scale);
    UI_SPRITE_CFG_INIT_ACTION(UI_SPRITE_ANIM_CAMERA_SHAKE_NAME, camera_shake);
	UI_SPRITE_CFG_INIT_ACTION(UI_SPRITE_ANIM_CAMERA_IN_AREA_NAME, camera_in_area);
    UI_SPRITE_CFG_INIT_ACTION(UI_SPRITE_ANIM_CAMERA_WAIT_STOP_NAME, camera_wait_stop);
    UI_SPRITE_CFG_INIT_ACTION(UI_SPRITE_TOUCH_MOVE_NAME, touch_move);
    UI_SPRITE_CFG_INIT_ACTION(UI_SPRITE_TOUCH_CLICK_NAME, touch_click);
    UI_SPRITE_CFG_INIT_ACTION(UI_SPRITE_CTRL_CIRCLE_NAME, ctrl_circle);

    return 0;
}

