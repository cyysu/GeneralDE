#ifndef CPE_POM_GRP_OBJ_H
#define CPE_POM_GRP_OBJ_H
#include "cpe/utils/bitarry.h"
#include "pom_grp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

pom_grp_obj_t
pom_mgr_obj_alloc(pom_grp_obj_mgr_t mgr);

void pom_grp_obj_free(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj);

uint16_t pom_grp_obj_page_count(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj);

/*normal data ops*/
uint16_t pom_grp_obj_nromal_capacity(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry);
void * pom_grp_obj_normal(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry);
void * pom_grp_obj_normal_check_or_create(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry);
int pom_grp_obj_normal_set(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry, void * data);

uint16_t pom_grp_obj_normal_capacity_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry);
void * pom_grp_obj_normal_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry);
void * pom_grp_obj_normal_check_or_create_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry);
int pom_grp_obj_normal_set_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry, void * data);

/*list data ops*/
uint16_t pom_grp_obj_list_count(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry);
void * pom_grp_obj_list_at(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry, uint16_t pos);
int pom_grp_obj_list_append(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry, void * data);
int pom_grp_obj_list_insert(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry, uint16_t pos, void * data);
int pom_grp_obj_list_remove(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry, uint16_t pos);
int pom_grp_obj_list_sort(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry, int (*cmp)(void const *, void const *));
void * pom_grp_obj_list_bsearch(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry, const void * key, int (*cmp)(void const *, void const *));

uint16_t pom_grp_obj_list_count_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry);
void * pom_grp_obj_list_at_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry, uint16_t pos);
int pom_grp_obj_list_append_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry, void * data);
int pom_grp_obj_list_insert_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry, uint16_t pos, void * data);
int pom_grp_obj_list_remove_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry, uint16_t pos);
int pom_grp_obj_list_sort_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry, int (*cmp)(void const *, void const *));
void * pom_grp_obj_list_bsearch_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry, const void * key, int (*cmp)(void const *, void const *));

/*ba data ops*/
uint16_t pom_grp_obj_ba_bit_capacity(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry);
uint16_t pom_grp_obj_ba_byte_capacity(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry);
uint16_t pom_grp_obj_ba_bit_count(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry);
int pom_grp_obj_ba_set_all(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry, cpe_ba_value_t value);
int pom_grp_obj_ba_set(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry, uint32_t pos, cpe_ba_value_t value);
cpe_ba_value_t pom_grp_obj_ba_get(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry, uint32_t pos);

uint16_t pom_grp_obj_ba_bit_capacity_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry);
uint16_t pom_grp_obj_ba_byte_capacity_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry);
uint16_t pom_grp_obj_ba_bit_count_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry);
int pom_grp_obj_ba_set_all_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry, cpe_ba_value_t value);
int pom_grp_obj_ba_set_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry, uint32_t pos, cpe_ba_value_t value);
cpe_ba_value_t pom_grp_obj_ba_get_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry, uint32_t pos);

/*binary data ops*/
uint16_t pom_grp_obj_binary_capacity(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry);
void * pom_grp_obj_binary(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry);
void * pom_grp_obj_binary_check_or_create(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry);
int pom_grp_obj_binary_set(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, const char * entry, void * data);

uint16_t pom_grp_obj_binary_capacity_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry);
void * pom_grp_obj_binary_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry);
void * pom_grp_obj_binary_check_or_create_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry);
int pom_grp_obj_binary_set_ex(pom_grp_obj_mgr_t mgr, pom_grp_obj_t obj, pom_grp_entry_meta_t entry, void * data);

#ifdef __cplusplus
}
#endif

#endif
