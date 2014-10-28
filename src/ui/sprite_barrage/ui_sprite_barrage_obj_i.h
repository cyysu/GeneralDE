#ifndef UI_SPRITE_BARRAGE_OBJ_I_H
#define UI_SPRITE_BARRAGE_OBJ_I_H
#include "plugin/barrage/plugin_barrage_emitter.h"
#include "ui/sprite_barrage/ui_sprite_barrage_obj.h"
#include "ui_sprite_barrage_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef TAILQ_HEAD(ui_sprite_barrage_emitter_list, ui_sprite_barrage_emitter) ui_sprite_barrage_emitter_list_t;

struct ui_sprite_barrage_obj {
    ui_sprite_barrage_module_t m_module;
    ui_sprite_barrage_emitter_list_t m_emitters;
};

int ui_sprite_barrage_obj_regist(ui_sprite_barrage_module_t module);
void ui_sprite_barrage_obj_unregist(ui_sprite_barrage_module_t module);

#ifdef __cplusplus
}
#endif

#endif
