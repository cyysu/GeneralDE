#include <string>
#include <map>
#include "RAudioManager.h"
#include "cpe/pal/pal_stdio.h"
#include "gdpp/app/Log.hpp"
#include "gdpp/app/Application.hpp"
#include "render/cache/ui_cache_manager.h"
#include "render/cache/ui_cache_group.h"
#include "uipp/sprite_cfg/CfgLoader.hpp"
#include "uipp/sprite_fsm/Fsm.hpp"
#include "uipp/sprite_fsm/ActionVisitor.hpp"
#include "uipp/sprite_fsm/Action.hpp"
#include "uipp/sprite_fsm/ActionMeta.hpp"
#include "cpepp/cfg/Node.hpp"
#include "UIPhaseExt.hpp"
#include "UICenterExt.hpp"
#include "EnvExt.hpp"
#include "UIAction_ShowPage.hpp"

namespace UI { namespace App {

class UIPhaseImpl : public UIPhaseExt {
public:
    struct StrCmp {
        bool operator()(const char * l, const char * r) const { return strcmp(l, r) < 0; };
    };
 
    typedef ::std::map<const char *, UIPageProxyExt *, StrCmp> PageContainer;

    UIPhaseImpl(UICenterExt & center, Cpe::Cfg::Node const & config)
        : m_center(center)
        , m_name(config.name())
        , m_fps(config["fps"])
        , m_runingFsm(loadRuningFsm(center, config.name(), config["runing-fsm"]))
        , m_group(NULL)
    {
        try {
            ui_cache_manager_t cache_mgr = ui_cache_manager_find_nc(center.env().app(), NULL);
            if (cache_mgr == NULL) {
                APP_CTX_THROW_EXCEPTION(
                    center.env().app(), ::std::runtime_error, "ui_cache_manager not exist!");
            }

            m_group = ui_cache_group_create(cache_mgr, m_name.c_str());
            if (m_group == NULL) {
                APP_CTX_THROW_EXCEPTION(
                    center.env().app(), ::std::runtime_error, "ui_cache_manager create group %s fail!", m_name.c_str());
            }

            PageCollector collector(*this);
            m_runingFsm.visitActions(collector);
            loadUsingTextures(config["using-textures"]);
			loadUsingSFX(config["using-audios"]);
			loadPlayBGM(config["play-bgm"]);
        }
        catch(...) {
            removeRuningFsm();
            throw;
        }
    }

    ~UIPhaseImpl() {
        removeRuningFsm();
    }

    virtual const char * name(void) const {
        return m_name.c_str();
    }

    virtual ui_cache_group_t group(void) {
        return m_group;
    }

    virtual float fps(void) const {
        return m_fps;
    } 

    virtual Sprite::Fsm::Fsm const & runingFsm(void) const {
        return m_runingFsm;
    }

    virtual ::std::set< ::std::string> const & usingTextures(void) const {
        return m_usingTextures;
    }

	virtual ::std::set< ::std::string> const & usingSFX(void) const {
		return m_usingSFX;
	}

	virtual ::std::set< ::std::string> const & playBGM(void) const {
		return m_playBGM;
	}
	
    virtual UICenterExt & center(void) {
        return m_center;
    }

    virtual UICenterExt const & center(void) const {
        return m_center;
    }

private:
    class PageCollector : public Sprite::Fsm::ActionVisitor {
    public:
        PageCollector(UIPhaseImpl & phase)
            : m_phase(phase)
        {
        }

        virtual void onAction(Sprite::Fsm::Action & action) {
            if (strcmp(action.meta().name(), UIAction_ShowPage::NAME) == 0) {
                UIAction_ShowPage * showPage = (UIAction_ShowPage*)(action.data());
                UIPageProxyExt * usingPage = m_phase.m_center.findPage(showPage->pageName());
                if (usingPage == NULL) {
                    APP_CTX_THROW_EXCEPTION(
                        m_phase.m_center.env().app(), ::std::runtime_error,
                        "Phase %s: using page %s not exist!", m_phase.m_name.c_str(), showPage->pageName());
                }

                if (m_phase.m_usingPages.insert(PageContainer::value_type(usingPage->name(), usingPage)).second) {
                    if (m_phase.m_center.env().debug()) {
                        APP_CTX_INFO(m_phase.m_center.env().app(), "Phase %s: using-page %s", m_phase.m_name.c_str(), usingPage->name());
                    }
                }
            }
        }

        UIPhaseImpl & m_phase;
    };

    void loadUsingTextures(Cpe::Cfg::Node const & config) {
        Cpe::Cfg::NodeConstIterator childs = config.childs();

        while(Cpe::Cfg::Node const * c = childs.next()) {
            const char * texture = c->asString(NULL);
            if (texture == NULL) {
                APP_CTX_THROW_EXCEPTION(
                    m_center.env().app(), ::std::runtime_error,
                    "Phase %s: load textures: texture config format error!", m_name.c_str());
            }

            if (m_usingTextures.insert(texture).second) {
                if (m_center.env().debug()) {
                    APP_CTX_INFO(m_center.env().app(), "Phase %s: using-texture %s", m_name.c_str(), texture);
                }
            }
        }
    }

	void loadUsingSFX(Cpe::Cfg::Node const & config) {
		Cpe::Cfg::NodeConstIterator childs = config.childs();

		while(Cpe::Cfg::Node const * c = childs.next()) {
			const char * sfx = c->asString(NULL);
			if (sfx == NULL) {
				APP_CTX_THROW_EXCEPTION(
					m_center.env().app(), ::std::runtime_error,
					"Phase %s: load sfx: sfx config format error!", m_name.c_str());
			}

			if (m_usingSFX.insert(sfx).second) {
				if (m_center.env().debug()) {
					APP_CTX_INFO(m_center.env().app(), "Phase %s: using-sfx %s", m_name.c_str(), sfx);
				}
			}
		}
	}

	void loadPlayBGM(Cpe::Cfg::Node const & config) {
		Cpe::Cfg::NodeConstIterator childs = config.childs();

		while(Cpe::Cfg::Node const * c = childs.next()) {
			const char * bgm = c->asString(NULL);
			if (bgm == NULL) {
				APP_CTX_THROW_EXCEPTION(
					m_center.env().app(), ::std::runtime_error,
					"Phase %s: load sfx: sfx config format error!", m_name.c_str());
			}

			if (m_playBGM.insert(bgm).second) {
				if (m_center.env().debug()) {
					APP_CTX_INFO(m_center.env().app(), "Phase %s: play-bgm %s", m_name.c_str(), bgm);
				}
			}
		}
	}

    void removeRuningFsm(void) {
        char fsm_proto_name[128];
        snprintf(fsm_proto_name, sizeof(fsm_proto_name), "ui.phase.%s.runing", m_name.c_str());
        m_center.env().world().removeProto(fsm_proto_name);

        if (m_group) {
            ui_cache_group_free(m_group);
            m_group = NULL;
        }
    }

    static Sprite::Fsm::Fsm & loadRuningFsm(UICenterExt & center, const char * phase_name, Cpe::Cfg::Node const & config) {
        Env & env = center.env();

        char fsm_proto_name[128];
        snprintf(fsm_proto_name, sizeof(fsm_proto_name), "ui.phase.%s.runing", phase_name);

        Sprite::Fsm::Fsm * fsm =
            Sprite::Cfg::CfgLoader::instance(env.app())
            .tryLoadProtoFsm(env.world(), fsm_proto_name, (cfg_t)config);
        if (fsm == NULL) {
            APP_CTX_THROW_EXCEPTION(center.env().app(), ::std::runtime_error, "load phase %s: load runing fsm fail!", phase_name);
        }

        return *fsm;
    }

    UICenterExt & m_center;
    ::std::string m_name;
    float m_fps;
    Sprite::Fsm::Fsm & m_runingFsm;
    ui_cache_group_t m_group;
    PageContainer m_usingPages;
    ::std::set< ::std::string> m_usingTextures;
	::std::set< ::std::string> m_usingSFX;
	::std::set< ::std::string> m_playBGM;

friend class PageCollector;
};

UIPhase::~UIPhase() {
}

::std::auto_ptr<UIPhaseExt>
UIPhaseExt::create(UICenterExt & center, Cpe::Cfg::Node const & config) {
    return ::std::auto_ptr<UIPhaseExt>(new UIPhaseImpl(center, config));
}

}}

