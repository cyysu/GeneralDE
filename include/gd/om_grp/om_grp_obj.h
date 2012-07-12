#ifndef GD_OM_GRP_OBJ_H
#define GD_OM_GRP_OBJ_H
#include "om_grp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

om_grp_obj_t
om_mgr_obj_alloc(om_grp_obj_mgr_t mgr);

void om_grp_obj_free(om_grp_obj_mgr_t mgr, om_grp_obj_t obj);

/*normal data ops*/
void * om_grp_obj_normal(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, const char * entry);
void * om_grp_obj_normal_check_or_create(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, const char * entry);
int om_grp_obj_normal_set(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, const char * entry, void * data);

void * om_grp_obj_normal_ex(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, om_grp_entry_meta_t entry);
void * om_grp_obj_normal_check_or_create_ex(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, om_grp_entry_meta_t entry);
int om_grp_obj_normal_set_ex(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, om_grp_entry_meta_t entry, void * data);

/*normal data ops*/
uint16_t om_grp_obj_list_count(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, om_grp_entry_meta_t entry);
void * om_grp_obj_list_at(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, om_grp_entry_meta_t entry, uint16_t pos);

#ifdef __cplusplus
}
#endif

#endif
