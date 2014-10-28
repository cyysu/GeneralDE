#ifndef CPE_UTILS_XML_H
#define CPE_UTILS_XML_H
#include "cpe/pal/pal_types.h"
#include "cpe/utils/error.h"

#ifdef __cplusplus
extern "C" {
#endif

int cpe_xml_find_attr_long(long * result, const char * attr_name, int nb_attributes, const void * attributes, error_monitor_t em);
int cpe_xml_find_attr_bool(uint8_t * result, const char * attr_name, int nb_attributes, const void * attributes, error_monitor_t em);
int cpe_xml_find_attr_float(float * result, const char * attr_name, int nb_attributes, const void * attributes, error_monitor_t em);
const char * cpe_xml_find_attr_string(
    char * buff, size_t capacity, const char * attr_name, int nb_attributes, const void * attributes, error_monitor_t em);

int cpe_xml_read_value_bool(uint8_t * result, const char * data, size_t data_len);
int cpe_xml_read_value_long(long * result, const char * data, size_t data_len);
int cpe_xml_read_value_float(float * result, const char * data, size_t data_len);

#ifdef __cplusplus
}
#endif

#endif
