#ifndef UI_SPRITE_BARRAGE_EMITTER_I_H
#define UI_SPRITE_BARRAGE_EMITTER_I_H
#include "plugin/barrage/plugin_barrage_emitter.h"
#include "ui/sprite_barrage/ui_sprite_barrage_emitter.h"
#include "ui_sprite_barrage_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_barrage_emitter {
    ui_sprite_barrage_obj_t m_obj;
    TAILQ_ENTRY(ui_sprite_barrage_emitter) m_next_for_obj;
    const char * m_group;
    plugin_barrage_emitter_t m_emitter;
    uint8_t m_pos_base;
    UI_SPRITE_2D_PAIR m_pos;
};

#ifdef __cplusplus
}
#endif

#endif
