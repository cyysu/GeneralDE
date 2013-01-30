#ifndef SVR_FRIEND_AGENT_SVRLOGIC_H
#define SVR_FRIEND_AGENT_SVRLOGIC_H
#include "cpepp/nm/Object.hpp"
#include "gdpp/utils/System.hpp"
#include "usfpp/mongo_cli/System.hpp"
#include "FriendSvrSystem.hpp"

namespace Svr { namespace Friend {

class FriendSvrLogic : public Cpe::Nm::Object {
public:
    virtual Usf::Mongo::CliProxy & db(void) = 0;
    virtual ~FriendSvrLogic();

    static cpe_hash_string_t NAME;

    static FriendSvrLogic & instance(Gd::App::Application & app);
};

}}

#endif
