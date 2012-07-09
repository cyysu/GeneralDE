#include "gd/om_grp/om_grp_meta.h"
#include "om_grp_internal_types.h"

om_grp_meta_t om_grp_meta_create(mem_allocrator_t alloc, const char * name) {
}

void om_grp_meta_free(om_grp_meta_t) {
    
}

const char * om_grp_meta_name(om_grp_meta_t meta) {
    return meta->m_name;
}
