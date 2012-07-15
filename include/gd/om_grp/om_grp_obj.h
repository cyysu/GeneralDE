#ifndef GD_OM_GRP_OBJ_H
#define GD_OM_GRP_OBJ_H
#include "cpe/utils/bitarry.h"
#include "om_grp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

om_grp_obj_t
om_mgr_obj_alloc(om_grp_obj_mgr_t mgr);

void om_grp_obj_free(om_grp_obj_mgr_t mgr, om_grp_obj_t obj);

uint16_t om_grp_obj_page_count(om_grp_obj_mgr_t mgr, om_grp_obj_t obj);

/*normal data ops*/
uint16_t om_grp_obj_nromal_capacity(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, const char * entry);
void * om_grp_obj_normal(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, const char * entry);
void * om_grp_obj_normal_check_or_create(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, const char * entry);
int om_grp_obj_normal_set(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, const char * entry, void * data);

uint16_t om_grp_obj_normal_capacity_ex(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, om_grp_entry_meta_t entry);
void * om_grp_obj_normal_ex(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, om_grp_entry_meta_t entry);
void * om_grp_obj_normal_check_or_create_ex(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, om_grp_entry_meta_t entry);
int om_grp_obj_normal_set_ex(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, om_grp_entry_meta_t entry, void * data);

/*list data ops*/
uint16_t om_grp_obj_list_count(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, const char * entry);
void * om_grp_obj_list_at(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, const char * entry, uint16_t pos);
int om_grp_obj_list_append(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, const char * entry, void * data);
int om_grp_obj_list_insert(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, const char * entry, uint16_t pos, void * data);
int om_grp_obj_list_remove(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, const char * entry, uint16_t pos);
int om_grp_obj_list_sort(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, const char * entry, int (*cmp)(void const *, void const *));
void * om_grp_obj_list_bsearch(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, const char * entry, const void * key, int (*cmp)(void const *, void const *));

uint16_t om_grp_obj_list_count_ex(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, om_grp_entry_meta_t entry);
void * om_grp_obj_list_at_ex(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, om_grp_entry_meta_t entry, uint16_t pos);
int om_grp_obj_list_append_ex(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, om_grp_entry_meta_t entry, void * data);
int om_grp_obj_list_insert_ex(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, om_grp_entry_meta_t entry, uint16_t pos, void * data);
int om_grp_obj_list_remove_ex(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, om_grp_entry_meta_t entry, uint16_t pos);
int om_grp_obj_list_sort_ex(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, om_grp_entry_meta_t entry, int (*cmp)(void const *, void const *));
void * om_grp_obj_list_bsearch_ex(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, om_grp_entry_meta_t entry, const void * key, int (*cmp)(void const *, void const *));

/*ba data ops*/
uint16_t om_grp_obj_ba_bit_capacity(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, const char * entry);
uint16_t om_grp_obj_ba_byte_capacity(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, const char * entry);
uint16_t om_grp_obj_ba_bit_count(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, const char * entry);
int om_grp_obj_ba_set_all(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, const char * entry, cpe_ba_value_t value);
int om_grp_obj_ba_set(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, const char * entry, uint32_t pos, cpe_ba_value_t value);
cpe_ba_value_t om_grp_obj_ba_get(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, const char * entry, uint32_t pos);

uint16_t om_grp_obj_ba_bit_capacity_ex(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, om_grp_entry_meta_t entry);
uint16_t om_grp_obj_ba_byte_capacity_ex(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, om_grp_entry_meta_t entry);
uint16_t om_grp_obj_ba_bit_count_ex(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, om_grp_entry_meta_t entry);
int om_grp_obj_ba_set_all_ex(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, om_grp_entry_meta_t entry, cpe_ba_value_t value);
int om_grp_obj_ba_set_ex(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, om_grp_entry_meta_t entry, uint32_t pos, cpe_ba_value_t value);
cpe_ba_value_t om_grp_obj_ba_get_ex(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, om_grp_entry_meta_t entry, uint32_t pos);

/*binary data ops*/
uint16_t om_grp_obj_binary_capacity(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, const char * entry);
void * om_grp_obj_binary(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, const char * entry);
void * om_grp_obj_binary_check_or_create(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, const char * entry);
int om_grp_obj_binary_set(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, const char * entry, void * data);

uint16_t om_grp_obj_binary_capacity_ex(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, om_grp_entry_meta_t entry);
void * om_grp_obj_binary_ex(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, om_grp_entry_meta_t entry);
void * om_grp_obj_binary_check_or_create_ex(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, om_grp_entry_meta_t entry);
int om_grp_obj_binary_set_ex(om_grp_obj_mgr_t mgr, om_grp_obj_t obj, om_grp_entry_meta_t entry, void * data);

#ifdef __cplusplus
}
#endif

#endif
