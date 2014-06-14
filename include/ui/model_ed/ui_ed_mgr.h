#ifndef UI_MODEL_ED_H
#define UI_MODEL_ED_H
#include "ui_ed_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_ed_mgr_t ui_ed_mgr_create(mem_allocrator_t alloc, ui_data_mgr_t data_mgr, error_monitor_t em);
void ui_ed_mgr_free(ui_ed_mgr_t);

int ui_ed_mgr_save(ui_ed_mgr_t mgr, const char * root, error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif
