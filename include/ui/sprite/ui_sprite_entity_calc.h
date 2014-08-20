#ifndef UI_SPRITE_ENTITYR_CALC_H
#define UI_SPRITE_ENTITYR_CALC_H
#include "cpe/utils/buffer.h"
#include "ui_sprite_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int8_t ui_sprite_entity_calc_bool_with_dft(const char * def, ui_sprite_entity_t entity, dr_data_source_t data_source, int8_t dft);
int64_t ui_sprite_entity_calc_int64_with_dft(const char * def, ui_sprite_entity_t entity, dr_data_source_t data_source, int64_t dft);
double ui_sprite_entity_calc_double_with_dft(const char * def, ui_sprite_entity_t entity, dr_data_source_t data_source, double dft);
const char * ui_sprite_entity_calc_str_with_dft(
    mem_buffer_t buffer, const char * def, ui_sprite_entity_t entity, dr_data_source_t data_source, const char * dft);

int ui_sprite_entity_try_calc_bool(int8_t * result, const char * def, ui_sprite_entity_t entity, dr_data_source_t data_source, error_monitor_t em);
int ui_sprite_entity_try_calc_int64(int64_t * result, const char * def, ui_sprite_entity_t entity, dr_data_source_t data_source, error_monitor_t em);
int ui_sprite_entity_try_calc_double(double * result, const char * def, ui_sprite_entity_t entity, dr_data_source_t data_source, error_monitor_t em);
const char * ui_sprite_entity_try_calc_str(
    mem_buffer_t buffer, const char * def, ui_sprite_entity_t entity, dr_data_source_t data_source, error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif
