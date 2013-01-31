#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_socket.h"
#include "cpepp/tl/Manager.hpp"
#include "cpepp/cfg/Node.hpp"
#include "cpepp/nm/Manager.hpp"
#include "gdpp/app/Application.hpp"
#include "gdpp/app/ModuleDef.hpp"
#include "gdpp/app/Log.hpp"
#include "gdpp/timer/TimerCenter.hpp"
#include "gdpp/timer/TimerProcessorBase.hpp"
#include "cpepp/cfg/System.hpp"
#include "cpepp/nm/Object.hpp"
#include "ChanelManager.hpp"
#include "Chanel.hpp"

namespace Svr { namespace Chat {

class ChanelTimer
    : public Cpe::Nm::Object
    , public Gd::Timer::TimerProcessorBase
{
public:
    static cpe_hash_string_t NAME;
    
    ChanelTimer(Gd::App::Application & app, Gd::App::Module & module, Cpe::Cfg::Node & cfg)
        : Gd::Timer::TimerProcessorBase(Gd::Timer::TimerCenter::instance(app))
        , m_app(app)
        , m_debug(cfg["debug"].dft(0))
        , m_once_save_count(cfg["once-save-count"].asInt32())
    {
        registerTimer(
            *this,
            &ChanelTimer::on_timer,
                cfg["timer-span-s"].asUInt32() * 1000,
                cfg["timer-span-s"].asUInt32() * 1000);
    }

    void on_timer(Gd::Timer::TimerID timerId) {
        int process_count;
        int max_process_count = m_once_save_count;

        ChanelManager & chanelManager = ChanelManager::instance(m_app);

        uint32_t curTimeS = m_app.tlManager().curTime() / 1000;

        if (max_process_count > (int)chanelManager.chanelCount()) {
            max_process_count = chanelManager.chanelCount();
        }

        for(process_count = 0; process_count < max_process_count; ++process_count) {
            Chanel * chanel = chanelManager.shift();
            if (chanel == NULL) {
                if (m_debug) {
                    APP_CTX_INFO(m_app, "%s: on_timer: no chanel, skip!", name());
                }
                continue;
            }

            SVR_CHAT_CHANEL_DATA & chanelData = chanel->data();

            uint32_t msg_remove_count = 0;
            while(chanelData.chanel_msg_r != chanelData.chanel_msg_w) {
                SVR_CHAT_MSG const & msg = chanel->msg(chanelData.chanel_msg_r);

                if (msg.send_time + chanelData.expire_time_s >= curTimeS) break;

                ++msg_remove_count;
                ++chanelData.chanel_msg_r;
                if (chanelData.chanel_msg_r >= chanelData.chanel_msg_capacity) {
                    chanelData.chanel_msg_r = 0;
                }
            }

            if (m_debug) {
                if (msg_remove_count > 0) {
                    APP_CTX_INFO(
                        m_app, "%s: on_timer: chanel %d."FMT_UINT64_T" remove %d messages!",
                        name(), chanelData.chanel_type, chanelData.chanel_id, msg_remove_count);
                }
            }

            if (chanelData.chanel_msg_r == chanelData.chanel_msg_w) {
                if (m_debug) {
                    APP_CTX_INFO(
                        m_app, "%s: on_timer: chanel %d."FMT_UINT64_T" no data, remove!",
                        name(), chanelData.chanel_type, chanelData.chanel_id);
                }
                chanelManager.removeChanel(chanel);
                continue;
            }
        }

        if (m_debug) {
            APP_CTX_INFO(m_app, "%s: on_timer: processed %d chanels!", name(), process_count);
        }
    }

private:
    Gd::App::Application & m_app;
    int m_debug;
    int m_once_save_count;
};

GDPP_APP_MODULE_DEF(ChanelTimer, ChanelTimer);

}}
