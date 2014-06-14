#ifndef UI_MODEL_DATA_MODULE_H
#define UI_MODEL_DATA_MODULE_H
#include "protocol/ui/model/ui_module.h"
#include "ui_model_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_data_img_block_it {
    ui_data_img_block_t (*next)(struct ui_data_img_block_it * it);
    char m_data[64];
};

ui_data_module_t ui_data_module_create(ui_data_mgr_t mgr, ui_data_src_t src);
void ui_data_module_free(ui_data_module_t module);

UI_MODULE * ui_data_module_data(ui_data_module_t module);
LPDRMETA ui_data_module_meta(ui_data_mgr_t data_mgr);

ui_data_img_block_t ui_data_img_block_create(ui_data_module_t module);
ui_data_img_block_t ui_data_img_block_find(ui_data_module_t module, uint32_t id);

int ui_data_img_block_set_id(ui_data_img_block_t block, uint32_t id);

void ui_data_img_block_free(ui_data_img_block_t img_block);
void ui_data_img_block_free_in_module(ui_data_module_t module);

void ui_data_img_block_in_module(ui_data_img_block_it_t it, ui_data_module_t module);
 
UI_IMG_BLOCK * ui_data_img_block_data(ui_data_img_block_t img_block);
LPDRMETA ui_data_img_block_meta(ui_data_mgr_t data_mgr);

#define ui_data_img_block_it_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif
