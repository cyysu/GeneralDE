#ifndef UI_MODEL_DATA_MGR_H
#define UI_MODEL_DATA_MGR_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "ui_model_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_data_mgr_t ui_data_mgr_create(mem_allocrator_t alloc, const char * root, error_monitor_t em);
void ui_data_mgr_free(ui_data_mgr_t mgr);

ui_data_src_t ui_data_mgr_src_root(ui_data_mgr_t mgr);

void ui_data_mgr_set_loader(ui_data_mgr_t mgr, ui_data_src_type_t type, product_load_fun_t loader, void * ctx);
void ui_data_mgr_set_saver(ui_data_mgr_t mgr, ui_data_src_type_t type, product_save_fun_t saver, product_remove_fun_t remover, void * ctx);

#ifdef __cplusplus
}
#endif

#endif 
