#ifndef USF_POM_GS_TESTENV_WITH_POM_GS_H
#define USF_POM_GS_TESTENV_WITH_POM_GS_H
#include "cpe/dr/dr_types.h"
#include "../pom_gs_agent.h"
#include "../pom_gs_pkg.h"
#include "../pom_gs_pkg_cfg.h"
#include "cpe/utils/tests-env/test-env.hpp"

namespace usf { namespace pom_gs { namespace testenv {

class with_pom_gs : public ::testenv::env<> {
public:
    with_pom_gs();

    void SetUp();
    void TearDown();

    const char * t_pom_gs_pkg_dump(pom_gs_pkg_t pkg);

    void t_pom_gs_pkg_dump_to_cfg(cfg_t cfg, pom_gs_pkg_t pkg);
    cfg_t t_pom_gs_pkg_dump_to_cfg(pom_gs_pkg_t pkg);

    void t_pom_gs_pkg_load(pom_gs_pkg_t pkg, cfg_t cfg);
    void t_pom_gs_pkg_load(pom_gs_pkg_t pkg, const char * cfg);
};

}}}

#endif
