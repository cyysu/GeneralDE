#ifndef USFPP_POM_GS_PKG_H
#define USFPP_POM_GS_PKG_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpe/dr/dr_data.h"
#include "cpepp/dr/Meta.hpp"
#include "gdpp/app/Application.hpp"
#include "usf/pom_gs/pom_gs_pkg.h"
#include "System.hpp"

namespace Usf { namespace Pom {

class PomGsPkg : public Cpe::Utils::SimulateObject {
public:
    operator pom_gs_pkg_t () const { return (pom_gs_pkg_t)this; }

    
};

}}

#endif
