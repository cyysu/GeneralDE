#ifndef UI_SPRITE_ANIM_SCH_I_H
#define UI_SPRITE_ANIM_SCH_I_H
#include "ui/sprite_anim/ui_sprite_anim_sch.h"
#include "ui_sprite_anim_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_anim_sch {
    ui_sprite_anim_backend_t m_backend;
    ui_sprite_anim_runing_list_t m_runings;
    ui_sprite_anim_def_list_t m_defs;
    ui_sprite_anim_group_list_t m_groups;
    ui_sprite_anim_template_list_t m_templates;
    char m_default_layer[64];
};

int ui_sprite_anim_sch_regist(ui_sprite_anim_module_t module);
void ui_sprite_anim_sch_unregist(ui_sprite_anim_module_t module);

#ifdef __cplusplus
}
#endif

#endif
