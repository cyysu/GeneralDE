#include "gdpp/app/ModuleDef.hpp"
#include "gdpp/app/Application.hpp"
#include "cpepp/nm/Manager.hpp"
#include "gdpp/utils/Gen/BasicMetaInfoManagerGen.hpp"
#include "ChanelInfoManager.hpp"

namespace Svr { namespace Chat {

struct ChanelInfoCompare {
    bool operator() (CHANELINFO const & l, CHANELINFO const & r) const {
        return l.chanel_type < r.chanel_type;
    }
};

class ChanelInfoManagerImpl
    : public Gd::Utils::BasicMetaInfoManagerGen<ChanelInfoManager, CHANELINFO, ChanelInfoCompare>
{
public:
    ChanelInfoManagerImpl(Gd::App::Application & app, Gd::App::Module & module, Cpe::Cfg::Node & cfg) {
        if (cfg["load"].dft(1)) {
            load(app.cfg()["meta.chanel_info"]);
        }

        if (cfg["debug"].dft(0)) {
            struct write_stream_file fs = CPE_WRITE_STREAM_FILE_INITIALIZER(stdout, NULL);
            dumpChanelInfos((write_stream_t)&fs);
        }
    }

    virtual CHANELINFO const * findChanelInfo(uint8_t chanel_type) const {
        CHANELINFO key;
        key.chanel_type = chanel_type;
        return find_first(key);
    }

private:
    void dumpChanelInfos(write_stream_t stream) const {
        stream_printf(stream, "======== dump chanel info table =======\n");

        dump(1, stream);

        stream_printf(stream, "======== dump chanel info table =======\n");
        stream_flush(stream);
    }
};

ChanelInfoManager::~ChanelInfoManager() {
}

ChanelInfoManager &
ChanelInfoManager::instance(Gd::App::Application & app) {
    ChanelInfoManager * r =
        dynamic_cast<ChanelInfoManager *>(
            &app.nmManager().object(ChanelInfoManager::NAME));
    if (r == NULL) {
        APP_THROW_EXCEPTION(::std::runtime_error, "ChanelInfoManager cast fail!");
    }
    return *r;
}

GDPP_APP_MODULE_DEF(ChanelInfoManager, ChanelInfoManagerImpl);

}}
