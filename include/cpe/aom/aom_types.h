#ifndef CPE_AOM_TYPES_H
#define CPE_AOM_TYPES_H
#include "cpe/pal/pal_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct aom_obj_mgr * aom_obj_mgr_t;

typedef struct aom_obj_it {
    void * (*next)(struct aom_obj_it * it);
    char m_data[64];
} * aom_obj_it_t;

#ifdef __cplusplus
}
#endif

#endif
