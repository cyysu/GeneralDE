#ifndef GD_OM_GRP_INTERNAL_OPS_H
#define GD_OM_GRP_INTERNAL_OPS_H
#include "om_grp_internal_types.h"

uint32_t om_grp_entry_meta_hash(const struct om_grp_entry_meta * entry_meta);
int om_grp_entry_meta_cmp(const struct om_grp_entry_meta * l, const struct om_grp_entry_meta * r);
void om_grp_entry_meta_free_all(om_grp_meta_t meta);

size_t om_grp_entry_meta_calc_bin_size(om_grp_meta_t meta);
om_grp_meta_t om_grp_entry_meta_build_from_bin(mem_allocrator_t alloc, void const * data, size_t data_capacity, LPDRMETALIB metalib, error_monitor_t em);
void om_grp_entry_meta_write_to_bin(void * data, size_t capacity, om_grp_meta_t meta);

#endif


