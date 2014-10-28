#ifndef UIPP_APP_ENV_EXT_H
#define UIPP_APP_ENV_EXT_H
#include "cpepp/utils/ObjHolder.hpp"
#include "cpepp/cfg/Node.hpp"
#include "uipp/app/Env.hpp"
#include "System.hpp"
#include "UICenterExt.hpp"
#include "DeviceExt.hpp"
#include "RuningExt.hpp"

namespace UI { namespace App {

class EnvExt : public Env {
public:
    EnvExt(Gd::App::Application & app, Gd::App::Module & module, Cpe::Cfg::Node & moduleCfg);
    ~EnvExt();

    virtual uint8_t debug(void) const;

    virtual Language language(void) const;
    virtual const char * appName(void) const;

    virtual Gd::App::Application & app(void);
    virtual Gd::App::Application const & app(void) const;

    virtual DeviceExt & device(void);
    virtual DeviceExt const & device(void) const;

    virtual RuningExt & runing(void);
    virtual RuningExt const & runing(void) const;

    virtual UICenterExt & uiCenter(void);
    virtual UICenterExt const & uiCenter(void) const;

    virtual Sprite::World & world(void);
    virtual Sprite::World const & world(void) const;

    virtual Sprite::P2D::Pair const & screenSize(void) const;
    virtual Sprite::P2D::Pair const & screenBaseSize(void) const;

    virtual const char * deviceId(void) const;
    virtual const char * documentPath(void) const;

    virtual const char * visiableMsg(uint32_t msg_id) const;
    virtual const char * visiableMsg(uint32_t msg_id, char * args) const;

    StringInfoMgr & stringInfoMgr(void) { return m_stringInfoMgr; }
    StringInfoMgr const & stringInfoMgr(void) const { return m_stringInfoMgr; }

    static EnvExt & instance(Gd::App::Application & app);

private:
    void doFini(void);
    void registerEvents(Sprite::Repository & repo, Cpe::Cfg::Node const & config);
    void loadModel(Cpe::Cfg::Node const & config);

    Language detectLanguage(void) const;

    Gd::App::Application & m_app;
    Cpe::Utils::ObjHolder<SpritePlugin> m_spritePlugin;
    Cpe::Utils::ObjHolder<UICenterExt> m_uiCenter;
    Cpe::Utils::ObjHolder<StringInfoMgr> m_stringInfoMgr;
    Cpe::Utils::ObjHolder<DeviceExt> m_device;
    Cpe::Utils::ObjHolder<RuningExt> m_runing;

    mutable Sprite::P2D::Pair m_screenSize;
    mutable Language m_language;
    mutable ::std::string m_deviceId;
    mutable ::std::string m_documentPath;

    Sprite::P2D::Pair m_screenBaseSize;
    ::std::string m_appName;
    uint8_t m_debug;

    ui_sprite_world_t m_world;
};

}}

#endif



