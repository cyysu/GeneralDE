#ifndef CPE_POM_GRP_INTERNAL_OPS_H
#define CPE_POM_GRP_INTERNAL_OPS_H
#include "pom_grp_internal_types.h"

uint32_t pom_grp_entry_meta_hash(const struct pom_grp_entry_meta * entry_meta);
int pom_grp_entry_meta_cmp(const struct pom_grp_entry_meta * l, const struct pom_grp_entry_meta * r);
void pom_grp_entry_meta_free_all(pom_grp_meta_t meta);

size_t pom_grp_entry_meta_calc_bin_size(pom_grp_meta_t meta);
pom_grp_meta_t pom_grp_entry_meta_build_from_bin(mem_allocrator_t alloc, void const * data, size_t data_capacity, LPDRMETALIB metalib, error_monitor_t em);
void pom_grp_entry_meta_write_to_bin(void * data, size_t capacity, pom_grp_meta_t meta);

#endif


