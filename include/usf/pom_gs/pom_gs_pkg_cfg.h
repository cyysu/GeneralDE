#ifndef USF_POM_GS_PKG_CFG_H
#define USF_POM_GS_PKG_CFG_H
#include "cpe/cfg/cfg_types.h"
#include "pom_gs_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int pom_gs_pkg_cfg_dump(cfg_t cfg, pom_gs_pkg_t pkg, error_monitor_t em);
int pom_gs_pkg_cfg_load(pom_gs_pkg_t pkg, cfg_t cfg, error_monitor_t em);

int pom_gs_pkg_dump_to_stream(write_stream_t stream, pom_gs_pkg_t pkg, error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif
