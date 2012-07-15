#ifndef CPE_POM_CLASS_H
#define CPE_POM_CLASS_H
#include "pom_types.h"

#ifdef __cplusplus
extern "C" {
#endif

pom_class_t pom_mgr_get_class(pom_mgr_t omm, pom_class_id_t);
pom_class_t pom_mgr_find_class(pom_mgr_t omm, cpe_hash_string_t className);

pom_class_id_t om_class_id(pom_class_t cls);
const char * pom_class_name(pom_class_t cls);
cpe_hash_string_t pom_class_name_hs(pom_class_t cls);
size_t pom_class_obj_size(pom_class_t cls);

#ifdef __cplusplus
}
#endif

#endif
