#ifndef UI_SPRITE_ANIM_CAMERA_RESTRICT_I_H
#define UI_SPRITE_ANIM_CAMERA_RESTRICT_I_H
#include "ui/sprite_anim/ui_sprite_anim_camera_restrict.h"
#include "ui_sprite_anim_camera_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_anim_camera_restrict {
    ui_sprite_anim_camera_t m_camera;
    const char * m_name;
    ui_sprite_anim_camera_restrict_fun_t m_fun;
    void * m_ctx;
    TAILQ_ENTRY(ui_sprite_anim_camera_restrict) m_next_for_camera;
};

#ifdef __cplusplus
}
#endif

#endif
