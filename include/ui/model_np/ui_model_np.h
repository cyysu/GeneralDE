#ifndef UI_MODEL_NP_H
#define UI_MODEL_NP_H
#include "../model/ui_model_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int ui_data_init_np_loader(ui_data_mgr_t mgr);
int ui_data_np_load(ui_data_mgr_t mgr, const char * dir, int load_product, error_monitor_t em);

int ui_data_init_np_saver(ui_data_mgr_t mgr);
int ui_data_np_save(ui_data_mgr_t mgr, const char * dir, error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif
