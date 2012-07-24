#ifndef CPE_POM_GRP_STORE_H
#define CPE_POM_GRP_STORE_H
#include "cpe/utils/buffer.h"
#include "pom_grp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int pom_grp_meta_build_store_meta(mem_buffer_t buffer, pom_grp_meta_t meta, error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif
