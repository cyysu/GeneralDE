#ifndef CPE_POM_DEBUGER_H
#define CPE_POM_DEBUGER_H
#include "cpe/utils/error.h"
#include "pom_types.h"

#ifdef __cplusplus
extern "C" {
#endif

pom_debuger_t
pom_debuger_enable(pom_mgr_t mgr, uint32_t stack_size, error_monitor_t em);
pom_debuger_t pom_debuger_get(pom_mgr_t mgr);

#ifdef __cplusplus
}
#endif

#endif
