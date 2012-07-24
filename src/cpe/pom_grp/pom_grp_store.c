#include "cpe/dr/dr_metalib_build.h"
#include "cpe/pom_grp/pom_grp_store.h"
#include "pom_grp_internal_types.h"

int pom_grp_meta_build_store_meta(mem_buffer_t buffer, pom_grp_meta_t meta, error_monitor_t em) {
    dr_metalib_builder_t builder = dr_metalib_builder_create(NULL, em);
    if (builder == NULL) {
        CPE_ERROR(em, "create metalib builder fail!");
        return -1;
    }

    return 0;
}
