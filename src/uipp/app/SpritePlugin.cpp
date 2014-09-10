#include "uipp/sprite/Repository.hpp"
#include "uipp/sprite_fsm/Repository.hpp"
#include "uipp/sprite_cfg/CfgLoaderExternGen.hpp"
#include "SpritePlugin.hpp"
#include "UIAction_ShowPage.hpp"
#include "UIAction_PlayAnim.hpp"
#include "UIAction_MoveInOut.hpp"
#include "EnvExt.hpp"
#include "UIAction_AudioBGM.hpp"
#include "UIAction_AudioSFX.hpp"
#include "UIAction_ScaleInOut.hpp"
#include "UIAction_AlphaInOut.hpp"
#include "UIAction_SwitchPhase.hpp"
#include "UIAction_SetupControl.hpp"

namespace UI { namespace App {

class SpritePluginImpl
    : public SpritePlugin
    , public Sprite::Cfg::CfgLoaderExternGen<SpritePluginImpl>
{
public:
    SpritePluginImpl(EnvExt & env)
        : m_env(env)
    {
        addActionLoader(&SpritePluginImpl::initShowPage);
        addActionLoader(&SpritePluginImpl::initPlayAnim);
		addActionLoader(&SpritePluginImpl::initAudioBGM);
		addActionLoader(&SpritePluginImpl::initAudioSFX);
		addActionLoader(&SpritePluginImpl::initMoveInOut);
		addActionLoader(&SpritePluginImpl::initScaleInOut);
		addActionLoader(&SpritePluginImpl::initAlphaInOut);
        addActionLoader(&SpritePluginImpl::initSwitchPhase);
        addActionLoader(&SpritePluginImpl::initSetupControl);

        Sprite::Fsm::Repository & fsm_repo = Sprite::Fsm::Repository::instance(env.app());
        UIAction_ShowPage::install(fsm_repo);
        UIAction_PlayAnim::install(fsm_repo);
		UIAction_AudioBGM::install(fsm_repo);
		UIAction_AudioSFX::install(fsm_repo);
		UIAction_MoveInOut::install(fsm_repo);
		UIAction_ScaleInOut::install(fsm_repo);
		UIAction_AlphaInOut::install(fsm_repo);
		UIAction_SetupControl::install(fsm_repo);
        UIAction_SwitchPhase::install(fsm_repo);
    }

    ~SpritePluginImpl() {
        Sprite::Fsm::Repository & fsm_repo = Sprite::Fsm::Repository::instance(m_env.app());

        fsm_repo.removeActionMeta<UIAction_SwitchPhase>();
		fsm_repo.removeActionMeta<UIAction_SetupControl>();
		fsm_repo.removeActionMeta<UIAction_AlphaInOut>();
		fsm_repo.removeActionMeta<UIAction_ScaleInOut>();
		fsm_repo.removeActionMeta<UIAction_MoveInOut>();
        fsm_repo.removeActionMeta<UIAction_ShowPage>();
        fsm_repo.removeActionMeta<UIAction_PlayAnim>();
		fsm_repo.removeActionMeta<UIAction_AudioSFX>();
		fsm_repo.removeActionMeta<UIAction_AudioBGM>();
    }

    Gd::App::Application & app(void) { return m_env.app(); }
    Gd::App::Application const & app(void) const { return m_env.app(); }

private:
    void initShowPage(UIAction_ShowPage & obj, Cpe::Cfg::Node const & cfg) const {
        obj.setEnv(m_env);
        obj.setPageName(cfg["page"]);
    }

    void initPlayAnim(UIAction_PlayAnim & obj, Cpe::Cfg::Node const & cfg) const {
        obj.setEnv(m_env);
        obj.setPageName(cfg["page"]);
        obj.setControlName(cfg["control"]);
        obj.setAnimName(cfg["anim"]);
    }

	void initAudioBGM(UIAction_AudioBGM & obj, Cpe::Cfg::Node const & cfg) const {
		obj.setRes(cfg["res"]);
		obj.setLoop(cfg_get_uint8(cfg, "loop", 1));
	}

	void initAudioSFX(UIAction_AudioSFX & obj, Cpe::Cfg::Node const & cfg) const {
		obj.setRes(cfg["res"]);
	}

	void initMoveInOut(UIAction_MoveInOut & obj, Cpe::Cfg::Node const & cfg) const {
		 obj.setEnv(m_env);
		 obj.setPageName(cfg["page"]);
		 obj.setControlName(cfg["control"]);
		 obj.setSpeed(cfg["speed"].dft(0.0f));
         obj.setTakeTime(cfg["take-time"].dft(0.0f));
		 obj.setPolicy(cfg["policy"]);
		 obj.setMoveWay(cfg["move-way"]);
		 if (const char * def = cfg["decorator"].asString(NULL)) {
			 obj.setDecotator(def);
		 }	 
	}

	void initScaleInOut(UIAction_ScaleInOut & obj, Cpe::Cfg::Node const & cfg) const {
		obj.setEnv(m_env);
		obj.setPageName(cfg["page"]);
		obj.setControlName(cfg["control"]);
		obj.setTakeTime(cfg["take-time"]);
		obj.setZoomWay(cfg["zoom-way"]);
		if (const char * def = cfg["decorator"].asString(NULL)) {
			obj.setDecotator(def);
		}	 
	}

	void initAlphaInOut(UIAction_AlphaInOut & obj, Cpe::Cfg::Node const & cfg) const {
		obj.setEnv(m_env);
		obj.setPageName(cfg["page"]);
		obj.setControlName(cfg["control"]);
        obj.setOriginAlpha(cfg["origin-alpha"].dft(0.0f));
		obj.setAlphaWay(cfg["way"]);
		obj.setTakeTime(cfg["take-time"]);
		if (const char * def = cfg["decorator"].asString(NULL)) {
			obj.setDecotator(def);
		}	 
	}

	void initSetupControl(UIAction_SetupControl & obj, Cpe::Cfg::Node const & cfg) const {
		obj.setEnv(m_env);
		obj.setPageName(cfg["page"]);
		obj.setControlName(cfg["control"]);
        obj.setVisiable(cfg["visiable"].asUInt8(obj.visiable()));
        obj.setRestore(cfg["restore"].asUInt8(obj.restore()));
	}

    void initSwitchPhase(UIAction_SwitchPhase & obj, Cpe::Cfg::Node const & cfg) const {
        obj.setEnv(m_env);
        obj.setPhaseName(cfg["phase"]);	 
    }
    
    EnvExt & m_env;
};

SpritePlugin::~SpritePlugin() {
}

::std::auto_ptr<SpritePlugin>
SpritePlugin::create(EnvExt & env) {
    return ::std::auto_ptr<SpritePlugin>(new SpritePluginImpl(env));
}

}}
