#ifndef UI_SPRITE_2D_MOVE_TO_FOLLOW_I_H
#define UI_SPRITE_2D_MOVE_TO_FOLLOW_I_H
#include "ui/sprite_2d/ui_sprite_2d_move_follow.h"
#include "ui_sprite_2d_module_i.h"
#include "ui/utils/ui_percent_decorator.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_2d_move_follow {
    ui_sprite_2d_module_t m_module;

    /*follow entity*/
    uint32_t m_follow_entity_id;
    char m_follow_entity_name[64];
    uint8_t m_follow_entity_pos;
    UI_SPRITE_2D_PAIR m_target_pos;

    float m_speed;
    float m_duration;
    struct ui_percent_decorator m_decorator;
};

int ui_sprite_2d_move_follow_regist(ui_sprite_2d_module_t module);
void ui_sprite_2d_move_follow_unregist(ui_sprite_2d_module_t module);

#ifdef __cplusplus
}
#endif

#endif
