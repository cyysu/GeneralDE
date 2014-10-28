#ifndef UI_SPRITE_SPINE_PLAY_ANIM_I_H
#define UI_SPRITE_SPINE_PLAY_ANIM_I_H
#include "render/spine/ui_spine_obj.h"
#include "ui/sprite_spine/ui_sprite_spine_play_anim.h"
#include "ui_sprite_spine_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_spine_play_anim {
    ui_sprite_spine_module_t m_module;
    char * m_anim_name;
    uint8_t m_is_loop;
    spTrackEntry * m_track_entry;
};

int ui_sprite_spine_play_anim_regist(ui_sprite_spine_module_t module);
void ui_sprite_spine_play_anim_unregist(ui_sprite_spine_module_t module);

#ifdef __cplusplus
}
#endif

#endif
