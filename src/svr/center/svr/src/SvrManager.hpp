#ifndef SVR_CENTER_SVR_SVRMANAGER_H
#define SVR_CENTER_SVR_SVRMANAGER_H
#include "gdpp/app/System.hpp"
#include "cpepp/nm/Object.hpp"
#include "CenterSvrSystem.hpp"
#include "protocol/svr/center/svr_center_internal.h"

namespace Svr { namespace Center {

class SvrManager : public Cpe::Nm::Object {
public:
    virtual ~SvrManager();

    static SvrManager & instance(Gd::App::Application & app);

    static cpe_hash_string_t NAME;
};

}}

#endif
