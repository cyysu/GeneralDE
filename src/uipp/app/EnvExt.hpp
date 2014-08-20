#ifndef UIPP_APP_ENV_EXT_H
#define UIPP_APP_ENV_EXT_H
#include "cpepp/utils/ObjHolder.hpp"
#include "cpepp/cfg/Node.hpp"
#include "uipp/app/Env.hpp"
#include "System.hpp"
#include "UICenterExt.hpp"

namespace UI { namespace App {

class EnvExt : public Env {
public:
    EnvExt(Gd::App::Application & app, Gd::App::Module & module, Cpe::Cfg::Node & moduleCfg);
    ~EnvExt();

    virtual uint8_t debug(void) const;

    virtual Gd::App::Application & app(void);
    virtual Gd::App::Application const & app(void) const;

    virtual UICenterExt & uiCenter(void);
    virtual UICenterExt const & uiCenter(void) const;

    virtual Sprite::World & world(void);
    virtual Sprite::World const & world(void) const;

    virtual Sprite::P2D::Pair const & screenSize(void) const;
    virtual const char * deviceId(void) const;

private:
    void doFini(void);
    void registerEvents(Sprite::Repository & repo, Cpe::Cfg::Node const & config);

    Gd::App::Application & m_app;
    Cpe::Utils::ObjHolder<SpritePlugin> m_spritePlugin;
    Cpe::Utils::ObjHolder<UICenterExt> m_uiCenter;

    mutable Sprite::P2D::Pair m_screenSize;
    mutable ::std::string m_deviceId;

    ui_sprite_world_t m_world;
    uint8_t m_debug;
};

}}

#endif



