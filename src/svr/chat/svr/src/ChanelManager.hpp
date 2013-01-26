#ifndef THE9_LMS_DATA_MANAGER_H
#define THE9_LMS_DATA_MANAGER_H
#include "cpe/utils/hash.h"
#include "gdpp/app/System.hpp"
#include "cpepp/nm/Object.hpp"
#include "cpepp/dr/System.hpp"
#include "protocol/svr/chat/svr_chat_pro.h"
#include "ChatSvrSystem.hpp"

namespace Svr { namespace Chat {

class ChanelManager : public Cpe::Nm::Object {
public:
    ChanelManager(
        Gd::App::Application & app,
        Gd::App::Module & module,
        Cpe::Cfg::Node & moduleCfg);

    virtual ~ChanelManager();

    int chanelCount(void) const;
    Chanel * findChanel(uint8_t chanel_type, uint64_t chanel_id);
    Chanel * createChanel(uint8_t chanel_type, uint64_t chanel_id);
    void removeChanel(Chanel * chanel);
    Chanel * shift(void);

    static ChanelManager & instance(Gd::App::Application & app);

    static cpe_hash_string_t NAME;

private:
    void chanel_queue_remove(Chanel * chanel);
    void chanel_queue_append(Chanel * chanel);

    Gd::App::Application & m_app;
    int m_debug;
    cpe_hash_table m_chanels;
    Chanel * m_chanel_queue_head;
    Chanel ** m_chanel_queue_tail;
};

}}

#endif
