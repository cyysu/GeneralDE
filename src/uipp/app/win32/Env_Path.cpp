#include <windows.h>
#include "../EnvExt.hpp"

namespace UI { namespace App {

const char * EnvExt::documentPath(void) const {
    if (m_documentPath.empty()) {
        uint32_t length = (uint32_t)GetCurrentDirectoryA(0, NULL);
        assert(length > 0);

        m_documentPath.resize(length + 1);

		GetCurrentDirectoryA(length + 1, &m_documentPath[0]);
    }

    return m_documentPath.c_str();
}

}}