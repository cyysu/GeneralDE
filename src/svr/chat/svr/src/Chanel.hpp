#ifndef SVR_CHAT_CHANEL_H
#define SVR_CHAT_CHANEL_H
#include "cpe/utils/hash.h" 
#include "protocol/svr/chat/svr_chat_internal_data.h"
#include "ChatSvrSystem.hpp"

namespace Svr { namespace Chat {

struct Chanel {
public:
    SVR_CHAT_MSG & appendMsg(void);
    SVR_CHAT_CHANEL_DATA & data(void) { return *m_data; }
    SVR_CHAT_CHANEL_DATA const & data(void) const { return *m_data; }

    SVR_CHAT_MSG const & msg(uint32_t pos) const { return ((SVR_CHAT_MSG *)(m_data + 1))[pos]; }
    
    static uint32_t chanel_hash(Chanel const * chanel);
    static int chanel_eq(Chanel const * l, Chanel const * r);

private:
    SVR_CHAT_CHANEL_DATA * m_data;
    cpe_hash_entry m_hh;
    Chanel * m_next;
    Chanel ** m_pre;

friend class ChanelManager;
};

}}

#endif
