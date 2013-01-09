#include "cpe/pal/pal_queue.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/hash.h"
#include "cpepp/cfg/Node.hpp"
#include "gdpp/app/Application.hpp"
#include "gdpp/app/ModuleDef.hpp"
#include "gdpp/app/Log.hpp"
#include "ChatSvrChanelManager.hpp"

namespace Svr { namespace Chat {

class ChanelManagerImpl : public ChanelManager {
public:
    ChanelManagerImpl(
        Gd::App::Application & app,
        Gd::App::Module & module,
        Cpe::Cfg::Node & moduleCfg)
        : m_app(app)
        , m_debug(moduleCfg["debug"].dft(0))
    {
        // if (cpe_hash_table_init(
        //     &m_role_index,
        //     app.allocrator(),
        //     (cpe_hash_fun_t) record_hash,
        //     (cpe_hash_cmp_t) record_eq,
        //     CPE_HASH_OBJ2ENTRY(Record, m_hh),
        //     -1) != 0)
        // {
        //     APP_CTX_THROW_EXCEPTION(app, ::std::runtime_error, "init hash table fail!");
        // }

        // for(uint32_t i = 0; i < (sizeof(m_level_roles) / sizeof(m_level_roles[0])); ++i) {
        //     TAILQ_INIT(&m_level_roles[i]);
        // }
    }

    ~ChanelManagerImpl() {
        // struct cpe_hash_it record_it;
        // Record * record;

        // cpe_hash_it_init(&record_it, &m_role_index);

        // record = (Record*)cpe_hash_it_next(&record_it);
        // while (record) {
        //     Record * next = (Record*)cpe_hash_it_next(&record_it);

        //     TAILQ_REMOVE(&m_level_roles[record->m_level - 1], record, m_next);
        //     cpe_hash_table_remove_by_ins(&m_role_index, record);
            
        //     record = next;
        // }
    }

private:
    Gd::App::Application & m_app;
    int m_debug;
};

ChanelManager::~ChanelManager() {
}

ChanelManager &
ChanelManager::instance(Gd::App::Application & app) {
    ChanelManager * r =
        dynamic_cast<ChanelManager *>(
            &app.nmManager().object(ChanelManager::NAME));
    if (r == NULL) {
        APP_THROW_EXCEPTION(::std::runtime_error, "ChanelManager cast fail!");
    }

    return *r;
}


GDPP_APP_MODULE_DEF(ChanelManager, ChanelManagerImpl);

}}

