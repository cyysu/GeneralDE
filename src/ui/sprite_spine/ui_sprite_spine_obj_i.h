#ifndef UI_SPRITE_ANIM_SCH_I_H
#define UI_SPRITE_ANIM_SCH_I_H
#include "render/spine/ui_spine_obj.h"
#include "ui/sprite_spine/ui_sprite_spine_obj.h"
#include "ui_sprite_spine_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_spine_obj {
    char m_obj_path[128];
    ui_spine_obj_t m_obj;
    uint8_t m_debug_slots;
	uint8_t m_debug_bones;
    float m_time_scale;
};

int ui_sprite_spine_obj_regist(ui_sprite_spine_module_t module);
void ui_sprite_spine_obj_unregist(ui_sprite_spine_module_t module);

#ifdef __cplusplus
}
#endif

#endif
