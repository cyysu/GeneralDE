#ifndef UI_MODEL_TOO_OPS_H
#define UI_MODEL_TOO_OPS_H
#include "ui/model/ui_data_mgr.h"
#include "ui/model_np/ui_model_np.h"

int do_manip_model(ui_data_mgr_t data_mgr, const char * op_script, error_monitor_t em);
int do_convert_model(ui_data_mgr_t data_mgr, const char * to, const char * format, error_monitor_t em);

int do_cocos_module_import(
    ui_data_mgr_t data_mgr,
    const char * to_module,
    const char * plist, const char * pic, error_monitor_t em);

int do_cocos_effect_import(
    ui_data_mgr_t data_mgr,
    const char * to_effect, const char * to_module,
    const char * plist, const char * pic, uint8_t frame_duration,
    const char * frame_position, const char * frame_order,
    error_monitor_t em);

int do_cocos_particle_import(
    ui_data_mgr_t data_mgr,
    const char * to_particle,
    const char * plist, const char * pic, error_monitor_t em);

#endif
