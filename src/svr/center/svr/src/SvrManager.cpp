#include <vector>
#include "gdpp/app/ModuleDef.hpp"
#include "gdpp/app/Application.hpp"
#include "cpepp/nm/Manager.hpp"
#include "SvrManager.hpp"

namespace Svr { namespace Center {

class SvrManagerImpl : public SvrManager {
public:
    SvrManagerImpl(Gd::App::Application & app, Gd::App::Module & module, Cpe::Cfg::Node & cfg) {
    }

private:
    ::std::vector<SVR_CENTER_SVR> m_svrs;
};

SvrManager::~SvrManager() {
}

SvrManager &
SvrManager::instance(Gd::App::Application & app) {
    SvrManager * r =
        dynamic_cast<SvrManager *>(
            &app.nmManager().object(SvrManager::NAME));
    if (r == NULL) {
        APP_THROW_EXCEPTION(::std::runtime_error, "SvrManager cast fail!");
    }
    return *r;
}

GDPP_APP_MODULE_DEF(SvrManager, SvrManagerImpl);

}}
