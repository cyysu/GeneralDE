#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_context.h"
#include "gdpp/app/Log.hpp"
#include "usfpp/pom_gs/PomGsPkg.hpp"
#include "usfpp/pom_gs/PomGsAgent.hpp"

namespace Usf { namespace Pom {

void PomGsPkg::setEntry(LPDRMETA meta, void const * data, size_t capacity, const char * entry_name) {
    if (pom_gs_pkg_set_entry(*this, entry_name, meta, data, capacity) != 0) {
        APP_CTX_THROW_EXCEPTION(
            agent().app(),
            ::std::runtime_error,
            "setEntry %s fail! meta=%s, capacity=%d", entry_name, dr_meta_name(meta), (int)capacity);
    }
}

}}
