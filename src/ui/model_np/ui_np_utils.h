#ifndef UI_NP_UTILS_H
#define UI_NP_UTILS_H
#include "ui/model/ui_model_types.h"

const char * ui_data_np_postfix(ui_data_src_type_t type);
const char * ui_data_np_control_tag_name(ui_data_control_type_t control_type);

const char * ui_data_np_particle_mod_type_name(uint8_t mod_type);
uint32_t ui_data_np_particle_mod_type_hash(uint8_t mod_type);
uint8_t ui_data_np_particle_mod_type(const char * mod_type_name);

#endif
