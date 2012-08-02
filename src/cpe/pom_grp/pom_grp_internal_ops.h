#ifndef CPE_POM_GRP_INTERNAL_OPS_H
#define CPE_POM_GRP_INTERNAL_OPS_H
#include "pom_grp_internal_types.h"

/*entry meta operations*/
uint32_t pom_grp_entry_meta_hash(const struct pom_grp_entry_meta * entry_meta);
int pom_grp_entry_meta_cmp(const struct pom_grp_entry_meta * l, const struct pom_grp_entry_meta * r);
void pom_grp_entry_meta_free_all(pom_grp_meta_t meta);

/*store table operations*/
uint32_t pom_grp_store_table_hash(const struct pom_grp_store_table * store_table);
int pom_grp_store_table_cmp(const struct pom_grp_store_table * l, const struct pom_grp_store_table * r);
void pom_grp_store_table_free_all(pom_grp_store_t store);
int pom_grp_store_table_build(pom_grp_store_t store, const char * main_entry, const char * key);

/*store entry operations*/
uint32_t pom_grp_store_entry_hash(const struct pom_grp_store_entry * store_entry);
int pom_grp_store_entry_cmp(const struct pom_grp_store_entry * l, const struct pom_grp_store_entry * r);
void pom_grp_store_entry_free_all(pom_grp_store_t store);
pom_grp_store_entry_t pom_grp_store_entry_create(pom_grp_store_table_t table, pom_grp_entry_meta_t entry_meta);
void pom_grp_store_entry_free(struct pom_grp_store_entry * store_entry);

#endif


