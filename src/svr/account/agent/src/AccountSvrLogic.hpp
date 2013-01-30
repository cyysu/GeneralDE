#ifndef SVR_ACCOUNT_AGENT_SVRLOGIC_H
#define SVR_ACCOUNT_AGENT_SVRLOGIC_H
#include "cpepp/nm/Object.hpp"
#include "gdpp/utils/System.hpp"
#include "usfpp/mongo_cli/System.hpp"
#include "AccountSvrSystem.hpp"

namespace Svr { namespace Account {

class AccountSvrLogic : public Cpe::Nm::Object {
public:
    virtual Usf::Mongo::CliProxy & db(void) = 0;
    virtual ~AccountSvrLogic();

    static cpe_hash_string_t NAME;

    static AccountSvrLogic & instance(Gd::App::Application & app);
};

}}

#endif
