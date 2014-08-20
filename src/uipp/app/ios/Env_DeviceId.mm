#include "../EnvExt.hpp"

namespace UI { namespace App {

const char * EnvExt::deviceId(void) const {
	return m_deviceId.c_str();
}

}}
