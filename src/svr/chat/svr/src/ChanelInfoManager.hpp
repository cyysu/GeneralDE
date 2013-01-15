#ifndef SVR_CHAT_SVR_CHANELINFOMAANGER_H
#define SVR_CHAT_SVR_CHANELINFOMAANGER_H
#include "gdpp/app/System.hpp"
#include "cpepp/nm/Object.hpp"
#include "protocol/svr/chat/chanel_info.h"
#include "ChatSvrSystem.hpp"

namespace Svr { namespace Chat {

class ChanelInfoManager : public Cpe::Nm::Object {
public:
    virtual CHANELINFO const * findChanelInfo(uint8_t chanel_type) const = 0;

    virtual void load(Cpe::Cfg::Node const & cfg) = 0;

    virtual ~ChanelInfoManager();

    static ChanelInfoManager & instance(Gd::App::Application & app);

    static cpe_hash_string_t NAME;
};

}}

#endif
