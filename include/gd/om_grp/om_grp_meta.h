#ifndef GD_OM_GRP_META_H
#define GD_OM_GRP_META_H
#include "cpe/dr/dr_types.h"
#include "om_grp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*meta operations*/
om_grp_meta_t om_grp_meta_create(mem_allocrator_t alloc, const char * name);
void om_grp_meta_free(om_grp_meta_t);

const char * om_grp_meta_name(om_grp_meta_t meta);

void om_grp_meta_dump(write_stream_t stream, om_grp_meta_t meta, int ident);

/*entry operations*/
om_grp_entry_meta_t
om_grp_entry_meta_normal_create(
    om_grp_meta_t meta,
    const char * entry_name,
    LPDRMETA entry_meta);

om_grp_entry_meta_t
om_grp_entry_meta_list_create(
    om_grp_meta_t meta,
    const char * entry_name,
    LPDRMETA entry_meta, uint32_t capacity);

om_grp_entry_meta_t
om_grp_entry_meta_ba_create(
    om_grp_meta_t meta,
    const char * entry_name,
    uint32_t capacity);

om_grp_entry_meta_t
om_grp_entry_meta_binary_create(
    om_grp_meta_t meta,
    const char * entry_name,
    uint32_t capacity);

void om_grp_entry_meta_free(om_grp_entry_meta_t entry_meta);

const char * om_grp_entry_meta_name(om_grp_entry_meta_t entry_meta);
om_grp_entry_type_t om_grp_entry_meta_type(om_grp_entry_meta_t entry_meta);

#ifdef __cplusplus
}
#endif

#endif
