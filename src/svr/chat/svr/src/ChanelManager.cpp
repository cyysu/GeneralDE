#include <cassert>
#include "cpe/pal/pal_queue.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/hash.h"
#include "cpepp/cfg/Node.hpp"
#include "gdpp/app/Application.hpp"
#include "gdpp/app/ModuleDef.hpp"
#include "gdpp/app/Log.hpp"
#include "ChanelManager.hpp"
#include "ChanelInfoManager.hpp"
#include "Chanel.hpp"

namespace Svr { namespace Chat {

ChanelManager::ChanelManager(
    Gd::App::Application & app,
    Gd::App::Module & module,
    Cpe::Cfg::Node & moduleCfg)
    : m_app(app)
    , m_debug(moduleCfg["debug"].dft(0))
    , m_chanel_queue_head(NULL)
    , m_chanel_queue_tail(&m_chanel_queue_head)
{
    if (cpe_hash_table_init(
        &m_chanels,
        app.allocrator(),
        (cpe_hash_fun_t) Chanel::chanel_hash,
        (cpe_hash_cmp_t) Chanel::chanel_eq,
        CPE_HASH_OBJ2ENTRY(Chanel, m_hh),
        -1) != 0)
    {
        APP_CTX_THROW_EXCEPTION(app, ::std::runtime_error, "init hash table fail!");
    }
}

ChanelManager::~ChanelManager() {
    struct cpe_hash_it chanel_it;
    Chanel * chanel;

    cpe_hash_it_init(&chanel_it, &m_chanels);

    chanel = (Chanel*)cpe_hash_it_next(&chanel_it);
    while (chanel) {
        Chanel * next = (Chanel*)cpe_hash_it_next(&chanel_it);

        removeChanel(chanel);

        chanel = next;
    }
}

int ChanelManager::chanelCount(void) const {
    return cpe_hash_table_count(const_cast<cpe_hash_table_t>(&m_chanels));
}

Chanel * ChanelManager::findChanel(uint8_t chanel_type, uint64_t chanel_id) {
    char key_buf(sizeof(Chanel) + sizeof(SVR_CHAT_CHANEL_DATA));
    Chanel * key = (Chanel*)key_buf;
    key->m_data = (SVR_CHAT_CHANEL_DATA *)(key + 1);

    key->m_data->chanel_type = chanel_type;
    key->m_data->chanel_id = chanel_id;

    return (Chanel*)cpe_hash_table_find(&m_chanels, key);
}

Chanel * ChanelManager::createChanel(uint8_t chanel_type, uint64_t chanel_id) {
    CHANELINFO const * chanelInfo = 
        ChanelInfoManager::instance(m_app)
        .findChanelInfo(chanel_type);
    if (chanelInfo == NULL) {
        APP_CTX_ERROR(m_app, "%s: create chanel: chanel_type %d not exist!", name(), chanel_type);
        return NULL;
    }
    
    Chanel * chanel = (Chanel*)new(std::nothrow) char[sizeof(Chanel) + sizeof(SVR_CHAT_CHANEL_DATA) + sizeof(SVR_CHAT_MSG) * chanelInfo->msg_expire_count];
    if (chanel == NULL) {
        APP_CTX_ERROR(m_app, "%s: create chanel: alloc fail!", name());
        return NULL;
    }

    chanel_queue_append(chanel);

    chanel->m_data = (SVR_CHAT_CHANEL_DATA *)(chanel + 1);

    chanel->m_data->chanel_id = chanel_id;
    chanel->m_data->chanel_type = chanel_type;
    chanel->m_data->chanel_sn = 0;
    chanel->m_data->expire_time_s = chanelInfo->msg_expire_time_s;
    chanel->m_data->chanel_msg_capacity = chanelInfo->msg_expire_count;
    chanel->m_data->chanel_msg_r = 0;
    chanel->m_data->chanel_msg_w = 0;

    return chanel;
}

void ChanelManager::removeChanel(Chanel * chanel) {
    chanel_queue_remove(chanel);
    cpe_hash_table_remove_by_ins(&m_chanels, chanel);
    delete [] (char*)chanel;
}

Chanel * ChanelManager::shift(void) {
    Chanel * r = m_chanel_queue_head;
    if (r == NULL) return NULL;

    chanel_queue_remove(r);
    chanel_queue_append(r);

    return r;
}

void ChanelManager::chanel_queue_remove(Chanel * chanel) {
    if (chanel->m_next) chanel->m_next->m_pre = chanel->m_pre;
    assert(chanel->m_pre);

    if (m_chanel_queue_tail == &chanel->m_next) {
        m_chanel_queue_tail = chanel->m_pre;
    }

    *chanel->m_pre = chanel->m_next;
}

void ChanelManager::chanel_queue_append(Chanel * chanel) {
    chanel->m_pre = m_chanel_queue_tail;
    chanel->m_next = NULL;
    *m_chanel_queue_tail = chanel;
    m_chanel_queue_tail = &chanel->m_next;
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

GDPP_APP_MODULE_DEF(ChanelManager, ChanelManager);

}}

