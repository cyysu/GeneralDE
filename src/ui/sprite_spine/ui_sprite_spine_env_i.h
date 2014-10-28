#ifndef UI_SPRITE_SPINE_ENV_I_H
#define UI_SPRITE_SPINE_ENV_I_H
#include "render/spine/ui_spine_obj_mgr.h"
#include "ui/sprite_spine/ui_sprite_spine_env.h"
#include "ui_sprite_spine_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_spine_env {
    ui_sprite_spine_module_t m_module;
    ui_spine_obj_mgr_t m_obj_mgr;
};

#ifdef __cplusplus
}
#endif

#endif
