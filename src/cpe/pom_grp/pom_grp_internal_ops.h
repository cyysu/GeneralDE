#ifndef CPE_POM_GRP_INTERNAL_OPS_H
#define CPE_POM_GRP_INTERNAL_OPS_H
#include "pom_grp_internal_types.h"

uint32_t pom_grp_entry_meta_hash(const struct pom_grp_entry_meta * entry_meta);
int pom_grp_entry_meta_cmp(const struct pom_grp_entry_meta * l, const struct pom_grp_entry_meta * r);
void pom_grp_entry_meta_free_all(pom_grp_meta_t meta);

#endif


