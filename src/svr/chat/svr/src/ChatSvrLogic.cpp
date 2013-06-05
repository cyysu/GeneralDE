#include "cpepp/cfg/Node.hpp"
#include "cpepp/nm/Object.hpp"
#include "cpepp/nm/Manager.hpp"
#include "gdpp/app/Application.hpp"
#include "gdpp/app/ModuleDef.hpp"
#include "gdpp/app/Log.hpp"
<<<<<<< Updated upstream
#include "usf/bpg_pkg/bpg_pkg.h"
=======
#include "usf/bpg_pkg/bpg_pkg_data.h"
>>>>>>> Stashed changes
#include "usf/bpg_rsp/bpg_rsp_manage.h"
#include "usf/bpg_rsp/bpg_rsp_addition.h"
#include "usfpp/bpg_rsp/RspManager.hpp"
#include "ChatSvrSystem.hpp"

extern "C" char g_metalib_svr_chat_pro[];

namespace Svr { namespace Chat {

class ChatSvrLogic : public Cpe::Nm::Object {
public:
    static cpe_hash_string_t NAME;

    ChatSvrLogic(Gd::App::Application & app, Gd::App::Module & module, Cpe::Cfg::Node & cfg)
        : m_rspManager(Usf::Bpg::RspManager::instance(app, cfg["rsp-manage"].asString(NULL)))
    {
        init_rsp_mgr(app, module, cfg);
    }

    ~ChatSvrLogic() {
        fini_rsp_manage();
    }

private:
    void init_rsp_mgr(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
        bpg_rsp_manage_set_context_op(
            m_rspManager,
            0,
            NULL,
            NULL,
            ChatSvr_pkg_init,
            NULL);
    }

    void fini_rsp_manage(void) {
        bpg_rsp_manage_set_context_op(
            m_rspManager,
            0,
            NULL,
            NULL,
            NULL,
            NULL);
    }

<<<<<<< Updated upstream
    static int ChatSvr_pkg_init(logic_context_t context, bpg_pkg_t pkg, void * ctx) {
=======
    static int ChatSvr_pkg_init(logic_context_t context, dp_req_t pkg, void * ctx) {
>>>>>>> Stashed changes
        bpg_pkg_set_cmd(pkg, bpg_pkg_cmd(pkg) + 1);
        bpg_rsp_addition_data_add_all(context);
        return 0;
    }

    Usf::Bpg::RspManager & m_rspManager;
};

GDPP_APP_MODULE_DEF(ChatSvrLogic, ChatSvrLogic);

}}
