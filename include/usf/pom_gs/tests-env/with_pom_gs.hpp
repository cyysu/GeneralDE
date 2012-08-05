#ifndef USF_POM_GS_TESTENV_WITH_POM_GS_H
#define USF_POM_GS_TESTENV_WITH_POM_GS_H
#include "cpe/dr/dr_types.h"
#include "../pom_gs_agent.h"
#include "../pom_gs_pkg.h"
#include "cpe/utils/tests-env/test-env.hpp"

namespace usf { namespace pom_gs { namespace testenv {

class with_pom_gs : public ::testenv::env<> {
public:
    with_pom_gs();

    void SetUp();
    void TearDown();
};

}}}

#endif
