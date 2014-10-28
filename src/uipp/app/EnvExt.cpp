#include "m3e.h"
#include "RRender.h"
#include "RFTDraw.h"
#include "R2DSLib.h"
#include "RGUILib.h"
#include "RAudio.h"
#include "RInput.h"
#include "RSystem.h"
#include "cpepp/nm/Manager.hpp"
#include "gdpp/app/Log.hpp"
#include "gdpp/app/ModuleDef.hpp"
#include "gd/dr_store/dr_store.h"
#include "gd/dr_store/dr_store_manage.h"
#include "render/model/ui_data_mgr.h"
#include "render/model_bin/ui_bin_loader.h"
#include "render/cache/ui_cache_manager.h"
#include "uipp/sprite/World.hpp"
#include "uipp/sprite/Repository.hpp"
#include "EnvExt.hpp"
#include "UICenterExt.hpp"
#include "SpritePlugin.hpp"
#include "StringInfoMgr.hpp"

namespace UI { namespace App {

EnvExt::EnvExt(Gd::App::Application & app, Gd::App::Module & module, Cpe::Cfg::Node & moduleCfg)
    : m_app(app)
    , m_stringInfoMgr(::std::auto_ptr<StringInfoMgr>(new StringInfoMgr(*this)))
    , m_device(::std::auto_ptr<DeviceExt>(new DeviceExt()))
    , m_runing(::std::auto_ptr<RuningExt>(new RuningExt(*this)))
    , m_language((Language)0)
    , m_debug(moduleCfg["debug"].dft((uint8_t)0))
    , m_world(NULL)
{
    m3eFileInit();

    RConfig::LoadConfig( "res.cfg" );
    RRender::Init();
    R2DSLib::Init();
    RFTDraw::Init();
    RAudio::Init();
    RGUILib::Init();
    RInput::Init();

    m_screenSize.x = 0.0f;
    m_screenSize.y = 0.0f;

    Cpe::Cfg::Node const & cfg = app.cfg()[moduleCfg["ui-load-from"].dft("ui")];

    m_screenBaseSize.x = cfg["env.base-size.w"];
    m_screenBaseSize.y = cfg["env.base-size.h"];
    m_appName = cfg["env.app-name"].asString();

    try {
        loadModel(cfg["env.ui-model"]);

        Sprite::Repository & repo = Sprite::Repository::instance(m_app);

        m_world = Sprite::World::create(repo);
        registerEvents(repo, cfg["env.events"]);

        m_spritePlugin.reset(SpritePlugin::create(*this));

        m_uiCenter.reset(UICenterExt::create(*this, cfg));
    }
    catch(...) {
        doFini();
        throw;
    }
}

EnvExt::~EnvExt() {
    doFini();
}

uint8_t EnvExt::debug(void) const {
    return m_debug;
}

Language EnvExt::language(void) const {
    if (m_language == 0) {
        m_language = detectLanguage();
    }

    return m_language;
}

const char * EnvExt::appName(void) const {
    return m_appName.c_str();
}

Gd::App::Application & EnvExt::app(void) {
    return m_app;
}

Gd::App::Application const & EnvExt::app(void) const {
    return m_app;
}

UICenterExt & EnvExt::uiCenter(void) {
    return m_uiCenter;
}

UICenterExt const & EnvExt::uiCenter(void) const {
    return m_uiCenter;
}

DeviceExt & EnvExt::device(void) {
    return m_device;
}

DeviceExt const & EnvExt::device(void) const {
    return m_device;
}

RuningExt & EnvExt::runing(void) {
    return m_runing;
}

RuningExt const & EnvExt::runing(void) const {
    return m_runing;
}

Sprite::World & EnvExt::world(void) {
    return *(Sprite::World*)m_world;
}

Sprite::World const & EnvExt::world(void) const {
    return *(Sprite::World const *)m_world;
}
    
Sprite::P2D::Pair const & EnvExt::screenSize(void) const {
    if (m_screenSize.x == 0.0f) {
        m_screenSize.x = (float)RRender::GetIns()->GetResolutionW();
        m_screenSize.y = (float)RRender::GetIns()->GetResolutionH();
    }

    return m_screenSize;
}

Sprite::P2D::Pair const & EnvExt::screenBaseSize(void) const {
    return m_screenBaseSize;
}

const char * EnvExt::visiableMsg(uint32_t msg_id) const {
    return m_stringInfoMgr.get().message(msg_id);
}

const char * EnvExt::visiableMsg(uint32_t msg_id, char * args) const {
    return m_stringInfoMgr.get().message(msg_id, args);
}

void EnvExt::registerEvents(Sprite::Repository & repo, Cpe::Cfg::Node const & config) {
    dr_store_manage_t store_mgr = dr_store_manage_default(app());
    if (store_mgr == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error,
            "%s: registerEvents: store_mgr not exist", name());
    }

    Cpe::Cfg::NodeConstIterator childs = config.childs();
    while(Cpe::Cfg::Node const * child = childs.next()) {
        const char * metalib = (*child)["metalib"].asString(NULL);

        if (metalib == NULL) {
            APP_CTX_THROW_EXCEPTION(
                app(), ::std::runtime_error,
                "%s: registerEvents: metalib not configured", name());
        }

        dr_store_t metalib_store = dr_store_find(store_mgr, metalib);
        if (metalib_store == NULL) {
            APP_CTX_THROW_EXCEPTION(
                app(), ::std::runtime_error,
                "%s: registerEvents: metalib %s not exist", name(), metalib);
        }

        if (const char * prefix = (*child)["prefix"].asString(NULL)) {
            repo.registerEventsByPrefix(dr_store_lib(metalib_store), prefix);
        }

        Cpe::Cfg::NodeConstIterator events = (*child)["events"].childs();
        if (Cpe::Cfg::Node const * event = events.next()) {
            const char * event_name = event->asString(NULL);
            if (event_name == NULL) {
                APP_CTX_THROW_EXCEPTION(
                    app(), ::std::runtime_error,
                    "%s: registerEvents: events format error", name());
            }

            LPDRMETA event_meta = dr_lib_find_meta_by_name(dr_store_lib(metalib_store), event_name);
            if (event_meta == NULL) {
                APP_CTX_THROW_EXCEPTION(
                    app(), ::std::runtime_error,
                    "%s: registerEvents: event %s not exist in metalib %s", name(), event_name, metalib);
            }

            repo.registerEvent(event_meta);
        }
    }
}

void EnvExt::doFini(void) {
    m_uiCenter.clear();

    if (m_world) {
        ui_sprite_world_free(m_world);
        m_world = NULL;
    }

    m_spritePlugin.clear();

    RFTDraw::ShutDown();	
    RGUILib::ShutDown();
    R2DSLib::ShutDown();
    RAudio::ShutDown();
    RInput::ShutDown();
    RRender::ShutDown();
}

void EnvExt::loadModel(Cpe::Cfg::Node const & config) {
    const char * data_mgr_name = config["data-mgr"].asString(NULL);
    ui_data_mgr_t data_mgr = ui_data_mgr_find_nc(m_app, data_mgr_name);
    if (data_mgr == NULL) {
        APP_CTX_THROW_EXCEPTION(
            m_app, ::std::runtime_error, "ui_data_mgr %s exist!", data_mgr_name ? data_mgr_name : "default");
    }

    const char * cache_manager_name = config["cache-mgr"].asString(NULL);
    ui_cache_manager_t cache_manager = ui_cache_manager_find_nc(m_app, cache_manager_name);
    if (cache_manager == NULL) {
        APP_CTX_THROW_EXCEPTION(
            m_app, ::std::runtime_error, "ui_cache_manager %s exist!", cache_manager_name ? cache_manager_name : "default");
    }

    if (const char * bin_loader_name = config["bin-loader"].asString(NULL)) {
        ui_bin_loader_t bin_loader = ui_bin_loader_find_nc(m_app, bin_loader_name);
        if (bin_loader == NULL) {
            APP_CTX_THROW_EXCEPTION(
                m_app, ::std::runtime_error, "ui_bin_loader %s exist!", bin_loader_name ? bin_loader_name : "default");
        }

        if (ui_data_bin_loader_load(bin_loader, data_mgr, 1) != 0) {
            APP_CTX_THROW_EXCEPTION(
                m_app, ::std::runtime_error, "load resource with ui_bin_loader fail!");
        }
    }
    else {
        APP_CTX_THROW_EXCEPTION(
            m_app, ::std::runtime_error, "no res load configured!");
    }
}

Env::~Env() {
}

Env &
Env::instance(Gd::App::Application & app) {
    Env * r =
        dynamic_cast<Env *>(
            &Gd::App::Application::instance().nmManager().object(Env::NAME));
    if (r == NULL) {
        APP_THROW_EXCEPTION(::std::runtime_error, "Env cast fail!");
    }

    return *r;
}

EnvExt &
EnvExt::instance(Gd::App::Application & app) {
    EnvExt * r =
        dynamic_cast<EnvExt *>(
            &Gd::App::Application::instance().nmManager().object(EnvExt::NAME));
    if (r == NULL) {
        APP_THROW_EXCEPTION(::std::runtime_error, "Env cast fail!");
    }

    return *r;
}

static cpe_hash_string_buf s_Env_Name = CPE_HS_BUF_MAKE("UIAppEnv");
cpe_hash_string_t Env::NAME = (cpe_hash_string_t)&s_Env_Name;

extern "C"
EXPORT_DIRECTIVE
int UIAppEnv_app_init(Gd::App::Application & app, Gd::App::Module & module, Cpe::Cfg::Node & moduleCfg) {                                                                   \
    Env * product = NULL;
    try {
        product = new (app.nmManager(), module.name()) EnvExt(app, module, moduleCfg);
        return 0;
    }
    APP_CTX_CATCH_EXCEPTION(app, "UIAppEnv init:");
    if (product) app.nmManager().removeObject(module.name());
    return -1;
}

extern "C"
EXPORT_DIRECTIVE
void UIAppEnv_app_fini(Gd::App::Application & app, Gd::App::Module & module) {
    app.nmManager().removeObject(module.name());
}

}}

