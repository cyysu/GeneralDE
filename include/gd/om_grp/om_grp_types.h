#ifndef GD_OM_GRP_TYPES_H
#define GD_OM_GRP_TYPES_H
#include "gd/om/om_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct om_grp_obj * om_grp_obj_t;
typedef struct om_grp_obj_mgr * om_grp_obj_mgr_t;

typedef enum om_grp_entry_type {
    om_grp_entry_type_normal
    , om_grp_entry_type_list
    , om_grp_entry_type_ba
    , om_grp_entry_type_binary
} om_grp_entry_type_t;

typedef struct om_grp_entry_meta * om_grp_entry_meta_t;
typedef struct om_grp_meta * om_grp_meta_t;

typedef struct om_grp_entry_meta_it {
    om_grp_entry_meta_t (*next)(struct om_grp_entry_meta_it * it);
    om_grp_entry_meta_t m_data;
} * om_grp_entry_meta_it_t;

#ifdef __cplusplus
}
#endif

#endif
