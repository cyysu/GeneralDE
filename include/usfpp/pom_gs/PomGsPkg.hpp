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

    PomGsAgent & agent(void) { return *(PomGsAgent*)pom_gs_pkg_agent(*this); }
    PomGsAgent const & agent(void) const { return *(PomGsAgent const *)pom_gs_pkg_agent(*this); }

    void setEntry(LPDRMETA meta, void const * data, size_t capacity, const char * entry_name = 0);

    template<typename T>
    void setEntry(T const & data, const char * entry_name = 0) {
        setData(Cpe::Dr::MetaTraits<T>::META, &data, sizeof(data), entry_name);
    }
};

}}

#endif
