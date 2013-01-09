#ifndef THE9_LMS_DATA_MANAGER_H
#define THE9_LMS_DATA_MANAGER_H
#include "gdpp/app/System.hpp"
#include "cpepp/nm/Object.hpp"
#include "cpepp/dr/System.hpp"
#include "protocol/svr_chat_pro.h"

namespace Svr { namespace Chat {

class ChanelManager : public Cpe::Nm::Object {
public:
    virtual ~ChanelManager();

    static ChanelManager & instance(Gd::App::Application & app);

    static cpe_hash_string_t NAME;
};

}}

#endif
