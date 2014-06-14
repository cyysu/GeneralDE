#ifndef UI_SPRITE_BASIC_TYPES_H
#define UI_SPRITE_BASIC_TYPES_H
#include "ui/sprite/ui_sprite_types.h"
#include "ui/sprite_fsm/ui_sprite_fsm_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_sprite_basic_module * ui_sprite_basic_module_t;

/*actions*/
typedef struct ui_sprite_basic_send_event * ui_sprite_basic_send_event_t;
typedef struct ui_sprite_basic_gen_entities * ui_sprite_basic_gen_entities_t;
typedef struct ui_sprite_basic_noop * ui_sprite_basic_noop_t;
typedef struct ui_sprite_basic_debug * ui_sprite_basic_debug_t;
typedef struct ui_sprite_basic_join_group * ui_sprite_basic_join_group_t;
typedef struct ui_sprite_basic_set_attrs * ui_sprite_basic_set_attrs_t;

#ifdef __cplusplus
}
#endif

#endif


