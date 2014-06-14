#ifndef UI_MODEL_DATA_LAYOUT_H
#define UI_MODEL_DATA_LAYOUT_H
#include "protocol/ui/model/ui_layout.h"
#include "ui_model_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_data_control_it {
    ui_data_control_t (*next)(struct ui_data_control_it * it);
    char m_data[64];
};

ui_data_layout_t ui_data_layout_create(ui_data_mgr_t mgr, ui_data_src_t src);
void ui_data_layout_free(ui_data_layout_t layout);

ui_data_control_t ui_data_layout_root(ui_data_layout_t layout);

/*control operations*/
ui_data_control_t ui_data_control_create(ui_data_layout_t layout, ui_data_control_t parent, ui_data_control_type_t type);
void ui_data_control_free(ui_data_control_t control);

ui_data_control_type_t ui_data_control_type(ui_data_control_t control);
LPDRMETA ui_data_control_meta(ui_data_mgr_t mgr, ui_data_control_type_t control_type);
void * ui_data_control_data(ui_data_control_t control);

ui_data_control_t ui_data_control_parent(ui_data_control_t control);
void ui_data_control_childs(ui_data_control_it_t it, ui_data_control_t control);

void ui_data_control_set_id(ui_data_control_t control, uint32_t id);
uint32_t ui_data_control_id(ui_data_control_t control);

ui_data_control_t ui_data_control_find_by_id(ui_data_layout_t layout, uint32_t id);

#define ui_data_control_it_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif
