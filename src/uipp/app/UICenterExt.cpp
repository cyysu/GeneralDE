#include <vector>
#include <map>
#include "RAudioManager.h"
#include "R2DSTextureCache.h"
#include "R2DSActorFileMgr.h"
#include "R2DSFrameFileMgr.h"
#include "R2DSImageFileMgr.h"
#include "cpe/pal/pal_stdio.h"
#include "cpepp/cfg/Node.hpp"
#include "uipp/sprite/World.hpp"
#include "uipp/sprite/Entity.hpp"
#include "gdpp/app/Log.hpp"
#include "gdpp/evt/EventCenter.hpp"
#include "gdpp/evt/EventResponserBase.hpp"
#include "UICenterExt.hpp"
#include "EnvExt.hpp"
#include "UIPhaseNodeExt.hpp"
#include "UIPopupPageDef.hpp"
#include "UIPopupPage.hpp"
#include "StringInfoMgr.hpp"
#include "protocol/ui/app/ui_app_evt.h"

namespace UI { namespace App {

class UICenterImpl
    : public UICenterExt
    , public Gd::Evt::EventResponserBase
{
public:
    struct StrCmp {
        bool operator()(const char * l, const char * r) const { return strcmp(l, r) < 0; };
    };
 
    typedef ::std::map<const char *, UIPageProxyExt *, StrCmp> PageContainer;
    typedef ::std::map<const char *, UIPhaseExt *, StrCmp> PhaseContainer;
    typedef ::std::vector<UIPhaseNodeExt *> PhaseNodeContainer;

    typedef ::std::map<const char *, UIPopupPageDef *, StrCmp> PopupPageDefContainer;
    typedef ::std::vector<UIPopupPage *> PopupPageContainer;

	typedef  ::std::map<const char *, const char *, StrCmp> ButtonAudioDefContainer;

    UICenterImpl(EnvExt & env, Cpe::Cfg::Node const & cfg)
        : Gd::Evt::EventResponserBase(Gd::Evt::EventCenter::instance(env.app()))
        , m_env(env)
        , m_init_phase(cfg["ui-center.init-phase"].asString())
        , m_error_msg_start(cfg["ui-center.error-msg-start"])
        , m_error_msg_dft(cfg["ui-center.error-msg-dft"])
        , m_error_msg_popup(cfg["ui-center.error-msg-popup"].dft("default"))
        , m_entity(env.world().createEntity("ui"))
    {
        m_entity.setDebug(env.debug());

        registerResponser("ui-center", *this, &UICenterImpl::onEvent);

        try {
            m_entity.enter();
			loadUIButtonAudio(cfg["ui-center.action-audio"]);
            loadPopupPageDefs(cfg["popups"]);
            loadPages(cfg["pages"]);
            loadPhases(cfg["phases"]);
        }
        catch(...) {
            doFini();
            throw;
        }
    }

    ~UICenterImpl() {
        doFini();
    }

    virtual Env & env(void) {
        return m_env;
    }

    virtual Env const & env(void) const {
        return m_env;
    }

    virtual UIPageProxyExt & page(const char * name) {
        UIPageProxyExt * r = findPage(name);
        if (r == NULL) {
            APP_CTX_THROW_EXCEPTION(m_env.app(), ::std::runtime_error, "page %s not exist", name);
        }
        return *r;
    }

    virtual UIPageProxyExt const & page(const char * name) const {
        UIPageProxyExt const * r = findPage(name);
        if (r == NULL) {
            APP_CTX_THROW_EXCEPTION(m_env.app(), ::std::runtime_error, "page %s not exist", name);
        }
        return *r;
    }

    virtual UIPageProxyExt * findPage(const char * name) {
        PageContainer::iterator pos = m_pages.find(name);
        return pos == m_pages.end() ? NULL : pos->second;
    }

    virtual UIPageProxyExt const * findPage(const char * name) const {
        PageContainer::const_iterator pos = m_pages.find(name);
        return pos == m_pages.end() ? NULL : pos->second;
    }

    virtual UIPhaseExt * findPhase(const char * name) {
        PhaseContainer::iterator pos = m_phases.find(name);
        return pos == m_phases.end() ? NULL : pos->second;
    }

    virtual UIPhaseExt const * findPhase(const char * name) const {
        PhaseContainer::const_iterator pos = m_phases.find(name);
        return pos == m_phases.end() ? NULL : pos->second;
    }

    virtual Sprite::Entity & entity(void) {
        return m_entity;
    }

    virtual Sprite::Entity const & entity(void) const {
        return m_entity;
    }

    virtual UIPhaseNodeExt & curentPhase(void) {
        if (m_phase_nodes.empty()) {
            APP_CTX_THROW_EXCEPTION(m_env.app(), ::std::runtime_error, "no curent phase");
        }

        return *m_phase_nodes.back();
    }

    virtual UIPhaseNode const & curentPhase(void) const {
        if (m_phase_nodes.empty()) {
            APP_CTX_THROW_EXCEPTION(m_env.app(), ::std::runtime_error, "no curent phase");
        }

        return *m_phase_nodes.back();
    }

    virtual void phaseSwitch(const char * phase_name) {
        UI_APP_EVT_SWITCH_PHASE evt;
        strncpy(evt.phase_name, phase_name, sizeof(evt.phase_name));
        eventCenter().sendEvent("ui-center", evt);
    }

    virtual void phaseCall(const char * phase_name) {
        UI_APP_EVT_CALL_PHASE evt;
        strncpy(evt.phase_name, phase_name, sizeof(evt.phase_name));
        eventCenter().sendEvent("ui-center", evt);
    }

    virtual void phaseBack(void) {
        if (m_phase_nodes.empty()) {
            APP_CTX_ERROR(m_env.app(), "pahse-back: no phase!!!");
            return;
        }

        if (m_phase_nodes.size() == 1) {
            APP_CTX_ERROR(m_env.app(), "pahse-back: only one phase, can`t back!!!");
            return;
        }

        UIPhaseNodeExt * returnToPhaseNode = m_phase_nodes[m_phase_nodes.size() - 2];
        UIPhaseNodeExt * curentPhaseNode = m_phase_nodes[m_phase_nodes.size() - 1];

        curentPhaseNode->exit();
        unloadPhaseResources(curentPhaseNode->phase(), &returnToPhaseNode->phase());

        if (returnToPhaseNode->enter()) {
            if (m_env.debug()) {
                APP_CTX_INFO(
                    m_env.app(), "phase-back: %s ==> %s: enter success",
                    curentPhaseNode->phase().name(), returnToPhaseNode->phase().name());
            }
            m_phase_nodes.pop_back();
            delete curentPhaseNode;
            return;
        }

        /*最后两个State都清理掉 */
        delete m_phase_nodes.back();
        m_phase_nodes.pop_back();

        delete m_phase_nodes.back();
        m_phase_nodes.pop_back();
        
        /*尝试回溯老状态 */
        while(!m_phase_nodes.empty()) {
            if (m_phase_nodes.back()->enter()) {
                APP_CTX_ERROR(
                    m_env.app(), "phase-back: re-enter phase %s success!", 
                    m_phase_nodes.back()->phase().name());
                return;
            }

            APP_CTX_ERROR(
                m_env.app(), "phase-back: re-enter phase %s fail!",
                m_phase_nodes.back()->phase().name());

            delete m_phase_nodes.back();
            m_phase_nodes.pop_back();
        }

        APP_CTX_ERROR(m_env.app(), "phase-back: no left phase!!!");
    }

    virtual void sendEvent(LPDRMETA meta, void const * data, size_t data_size) {
        m_entity.sendEvent(meta, data, data_size);
    }

    virtual void showPopupPage(const char * message, const char * template_name) {
        if (template_name == NULL) template_name = "default";

        PopupPageDefContainer::iterator defPos = m_popupPageDefs.find(template_name);
        if (defPos == m_popupPageDefs.end()) {
            APP_CTX_ERROR(
                m_env.app(), "popup page %s not exist!", template_name);
        }

        UIPopupPage * page = new UIPopupPage(*this, *defPos->second);
        assert(!m_popupPages.empty());
        assert(m_popupPages.back() == page);

        defPos->second->setData(m_env.app(), page, message, *this);
    }

    virtual void showPopupPage(LPDRMETA meta, void const * data, size_t data_size, const char * template_name) {
        if (template_name == NULL) template_name = "default";

        PopupPageDefContainer::iterator defPos = m_popupPageDefs.find(template_name);
        if (defPos == m_popupPageDefs.end()) {
            APP_CTX_ERROR(
                m_env.app(), "popup page %s not exist!", template_name);
        }

        UIPopupPage * page = new UIPopupPage(*this, *defPos->second);
        assert(!m_popupPages.empty());
        assert(m_popupPages.back() == page);

        defPos->second->setData(m_env.app(), page, meta, data, data_size, *this);
    }

    virtual void showPopupErrorMsg(int error, const char * template_name) {
        if (template_name == NULL) template_name = m_error_msg_popup.c_str();

        if (error < 0) {
            showPopupPage(
                m_env.stringInfoMgr().message(m_error_msg_dft),
                template_name);
        }
        else {
            showPopupPage(
                m_env.stringInfoMgr().message(error + m_error_msg_start),
                template_name);
        }
    }

    virtual void stopPopupPage(UIPopupPage * page) {
        delete page; 
    }

    virtual void addPopupPage(UIPopupPage & page) {
        m_popupPages.push_back(&page);
    }

    virtual void removePopupPage(UIPopupPage & page) {
        for(PopupPageContainer::iterator it = m_popupPages.begin();
            it != m_popupPages.end();
            ++it)
        {
            if (*it == &page) {
                m_popupPages.erase(it);
                return;
            }
        }

        assert(false);
    }

	const char * findAudioByPostfix(const char * postfix) const {
        ButtonAudioDefContainer::const_iterator pos = m_buttonAudioDefs.find(postfix);
        return pos == m_buttonAudioDefs.end() ? NULL : pos->second;
    }

private:
    void loadPages(Cpe::Cfg::Node const & config) {
        Cpe::Cfg::NodeConstIterator page_node_it = config.childs();

        while(Cpe::Cfg::Node const * page_node = page_node_it.next()) {
            ::std::auto_ptr<UIPageProxyExt> page = UIPageProxyExt::create(*this, *page_node);

            ::std::pair<PageContainer::iterator, bool>
                  insertResult = m_pages.insert(PageContainer::value_type(page->name(), page.get()));
            if (!insertResult.second) {
                APP_CTX_THROW_EXCEPTION(m_env.app(), ::std::runtime_error, "page %s duplicate!", page->name());
            }
            page.release();
        }
    }

    void loadPhases(Cpe::Cfg::Node const & config) {
        Cpe::Cfg::NodeConstIterator phase_node_it = config.childs();

        while(Cpe::Cfg::Node const * phase_node = phase_node_it.next()) {
            ::std::auto_ptr<UIPhaseExt> phase = UIPhaseExt::create(*this, *phase_node);

            ::std::pair<PhaseContainer::iterator, bool>
                  insertResult = m_phases.insert(PhaseContainer::value_type(phase->name(), phase.get()));
            if (!insertResult.second) {
                APP_CTX_THROW_EXCEPTION(m_env.app(), ::std::runtime_error, "phase %s duplicate!", phase->name());
            }
            phase.release();
        }
    }

	void loadUIButtonAudio(Cpe::Cfg::Node const & config) {
		Cpe::Cfg::NodeConstIterator audio_node_it = config.childs();
		
		while(Cpe::Cfg::Node const * audio_node = audio_node_it.next()) {
			m_buttonAudioDefs.insert(ButtonAudioDefContainer::value_type((*audio_node)["post-event"].asString(""),
				(*audio_node)["res"].asString("")));
		}
	}

    void loadPopupPageDefs(Cpe::Cfg::Node const & config) {
        Cpe::Cfg::NodeConstIterator popup_node_it = config.childs();

        while(Cpe::Cfg::Node const * popup_node = popup_node_it.next()) {
            ::std::auto_ptr<UIPopupPageDef> popup(new UIPopupPageDef(m_env.app(), *popup_node));

            ::std::pair<PopupPageDefContainer::iterator, bool>
                  insertResult = m_popupPageDefs.insert(PopupPageDefContainer::value_type(popup->name(), popup.get()));
            if (!insertResult.second) {
                APP_CTX_THROW_EXCEPTION(m_env.app(), ::std::runtime_error, "popup %s duplicate!", popup->name());
            }
            popup.release();
        }
    }

    void doFini(void) {
        for(PhaseNodeContainer::reverse_iterator it = m_phase_nodes.rbegin(); it != m_phase_nodes.rend(); ++it) {
            delete *it;
        }
        m_phase_nodes.clear();

        for(PhaseContainer::iterator it = m_phases.begin(); it != m_phases.end(); ++it) {
            delete it->second;
        }
        m_phases.clear();

        for(PageContainer::iterator it = m_pages.begin(); it != m_pages.end(); ++it) {
            delete it->second;
        }
        m_pages.clear();
    }

    void unloadPhaseResources(UIPhaseExt const & phase, UIPhaseExt const * nextPhase) {
        unloadTexturesExcept(nextPhase);
    }

    void loadPhaseResources(UIPhaseExt const & phase) {
        loadTextures(phase.usingTextures());
		loadSFX(phase.usingSFX());
		playBGM(phase.playBGM());
    }

    void loadTextures(::std::set< ::std::string> const & textures) {
        char buf[128];

        for(::std::set< ::std::string>::const_iterator it = textures.begin();
            it != textures.end();
            ++it)
        {
            int id = -1;
            snprintf(buf, sizeof(buf), "%s.pzd", it->c_str());
            char * sep = strrchr(buf, '/');
            if (sep) {
                *sep = 0;
                id = R2DSTextureCache::GetIns()->GetTextureID(buf, sep + 1, true );
            }
            else {
                id = R2DSTextureCache::GetIns()->GetTextureID("", buf, true );
            }

            if (id != -1) {
                if (m_env.debug()) {
                    APP_CTX_INFO(m_env.app(), "load texture %d(%s)", id, it->c_str());
                }
            }
            else {
                APP_CTX_ERROR(m_env.app(), "load texture %s fail", it->c_str());
            }
        }
    }


	void loadSFX(::std::set< ::std::string> const & sfx) {
		for(::std::set< ::std::string>::const_iterator it = sfx.begin();
			it != sfx.end();
			++it)
		{
			int id = -1;
			std::string filename = it->c_str();
			RAudioManager*	audioManager = RAudioManager::GetIns();
			assert(audioManager);
			id = audioManager->AddSFX( filename );

			if (id != -1) {
				if (m_env.debug()) {
					APP_CTX_INFO(m_env.app(), "load sfx %d(%s)", id, it->c_str());
				}
			}
			else {
				APP_CTX_ERROR(m_env.app(), "load sfx %s fail", it->c_str());
			}
		}
	}

	void playBGM(::std::set< ::std::string> const & bgm) {
		for(::std::set< ::std::string>::const_iterator it = bgm.begin();
			it != bgm.end();
			++it)
		{
			int id = -1;
			std::string filename = it->c_str();
			RAudioManager*	audioManager = RAudioManager::GetIns();
			assert(audioManager);
			id = audioManager->AddBGM( filename );
			if (id != -1) {
				audioManager->StopBGM();
				audioManager->PlayBGM(id, true);
			}
			else {
				APP_CTX_ERROR(m_env.app(), "play bgm %s fail", it->c_str());
			}
		}
	}

	void unloadTexturesExcept(UIPhaseExt const * exceptForPhase) {
		R2DSActorFileMgr::GetIns()->DelFileAll();
		R2DSFrameFileMgr::GetIns()->DelFileAll();
		R2DSImageFileMgr::GetIns()->DelFileAll();
		R2DSTextureCache* textureCache = R2DSTextureCache::GetIns();

		uint32_t textureCount = textureCache->GetTextureCount();

        char buf[128];
		for(uint32_t i = 0; i < textureCount; i++) {
			int id = textureCache->GetTextureID(i);
			if(id == -1) continue;

            if (exceptForPhase) {
                ::std::string const & path = textureCache->GetTexturePH(i);
                ::std::string const & file = textureCache->GetTextureFN(i);

                snprintf(buf, sizeof(buf), "%s%s", path.c_str(), file.c_str());
                if (char * sep = strrchr(buf, '.')) *sep = 0;

                if (exceptForPhase->usingTextures().find(buf) != exceptForPhase->usingTextures().end()) {
                    if (m_env.debug()) {
                        APP_CTX_INFO(m_env.app(), "unloand texture %d(%s) skip", id, buf);
                    }
                    continue;
                }
            }

            textureCache->DelTexture(id);
            if (m_env.debug()) {
                APP_CTX_INFO(
                    m_env.app(), "unloand texture %d(%s%s.pzd) complete",
                    id, textureCache->GetTexturePH(i).c_str(), textureCache->GetTextureFN(i).c_str());
            }
        }
	}

    void initPhase(void) {
        doPhaseSwitch(m_init_phase.c_str());
    }

    void onEvent(const char * oid, Gd::Evt::Event const & e) {
        if (e.type() == "ui_app_evt_switch_phase") {
            doPhaseSwitch(e.as<UI_APP_EVT_SWITCH_PHASE>().phase_name);
        }
        else if (e.type() == "ui_app_evt_call_phase") {
            doPhaseCall(e.as<UI_APP_EVT_CALL_PHASE>().phase_name);
        }
    }

    void doPhaseSwitch(const char * phase_name) {
        UIPhaseExt * phase = findPhase(phase_name);
        if (phase == NULL) {
            APP_CTX_THROW_EXCEPTION(
                m_env.app(), ::std::runtime_error,
                "switch to phase %s: phase not exist", phase_name);
        }

        ::std::auto_ptr<UIPhaseNodeExt> oldPhaseNode;
        if (!m_phase_nodes.empty()) {
			oldPhaseNode.reset(m_phase_nodes.back());
			m_phase_nodes.pop_back();

			oldPhaseNode->saveState();
			oldPhaseNode->exit();
			unloadPhaseResources(oldPhaseNode->phase(), phase);
        }

		::std::auto_ptr<UIPhaseNodeExt> newPhaseNode = UIPhaseNodeExt::create(*phase);
		m_phase_nodes.push_back(newPhaseNode.get());

		loadPhaseResources(newPhaseNode->phase());
        m_env.runing().setFps(newPhaseNode->phase().fps());
        m_env.world().setFps(newPhaseNode->phase().fps());

		if (newPhaseNode->enter()) {
			newPhaseNode.release();

            if (m_env.debug()) {
                APP_CTX_INFO(m_env.app(), "swith-to-pahse %s: enter success!", phase_name);
            }
            return;
        }

        APP_CTX_ERROR(m_env.app(), "swith-to-pahse %s: enter fail!", phase_name);

        /*清理掉新状态的资源 */
        unloadPhaseResources(newPhaseNode->phase(), NULL);
        m_phase_nodes.pop_back();
        newPhaseNode.reset(0);

        /*有老的状态，把愿状态添加回去 */
        if (oldPhaseNode.get()) {
            m_env.runing().setFps(oldPhaseNode->phase().fps());
            m_env.world().setFps(oldPhaseNode->phase().fps());
            m_phase_nodes.push_back(oldPhaseNode.get());
            oldPhaseNode.release();
        }

        /*尝试回溯老状态 */
        while(!m_phase_nodes.empty()) {
            if (m_phase_nodes.back()->enter()) {
                APP_CTX_ERROR(
                    m_env.app(), "swith-to-pahse %s: re-enter phase %s success!", 
                    phase_name, m_phase_nodes.back()->phase().name());
                return;
            }

            APP_CTX_ERROR(
                m_env.app(), "swith-to-pahse %s: re-enter phase %s fail!", 
                phase_name, m_phase_nodes.back()->phase().name());

            delete m_phase_nodes.back();
            m_phase_nodes.pop_back();
        }

        APP_CTX_ERROR(m_env.app(), "swith-to-pahse %s: no left phase!!!", phase_name);
    }

    void doPhaseCall(const char * phase_name) {
        UIPhaseExt * phase = findPhase(phase_name);
        if (phase == NULL) {
            APP_CTX_THROW_EXCEPTION(
                m_env.app(), ::std::runtime_error,
                "call to phase %s: phase not exist", phase_name);
        }

        UIPhaseNodeExt * oldPhaseNode = NULL;
        if (!m_phase_nodes.empty()) {
            oldPhaseNode = m_phase_nodes.back();
            oldPhaseNode->saveState();
            oldPhaseNode->exit();
            unloadPhaseResources(oldPhaseNode->phase(), phase);
        }

        ::std::auto_ptr<UIPhaseNodeExt> newPhaseNode = UIPhaseNodeExt::create(*phase);
        m_phase_nodes.push_back(newPhaseNode.get());

        loadPhaseResources(newPhaseNode->phase());
        if (newPhaseNode->enter()) {
            newPhaseNode.release();

            if (m_env.debug()) {
                APP_CTX_INFO(m_env.app(), "call-to-pahse %s: enter success!", phase_name);
            }
            return;
        }

        APP_CTX_ERROR(m_env.app(), "call-to-pahse %s: enter fail!", phase_name);

        /*清理掉新状态的资源 */
        unloadPhaseResources(newPhaseNode->phase(), oldPhaseNode ? &oldPhaseNode->phase() : NULL);
        m_phase_nodes.pop_back();
        newPhaseNode.reset(0);

        /*尝试回溯老状态 */
        while(!m_phase_nodes.empty()) {
            if (m_phase_nodes.back()->enter()) {
                APP_CTX_ERROR(
                    m_env.app(), "call-to-pahse %s: re-enter phase %s success!", 
                    phase_name, m_phase_nodes.back()->phase().name());
                return;
            }

            APP_CTX_ERROR(
                m_env.app(), "call-to-pahse %s: re-enter phase %s fail!", 
                phase_name, m_phase_nodes.back()->phase().name());

            delete m_phase_nodes.back();
            m_phase_nodes.pop_back();
        }

        APP_CTX_ERROR(m_env.app(), "call-to-pahse %s: no left phase!!!", phase_name);
    }

    EnvExt & m_env;
    ::std::string m_init_phase;
    uint32_t m_error_msg_start;
    uint32_t m_error_msg_dft;
    ::std::string m_error_msg_popup;
    Sprite::Entity & m_entity;
    PageContainer m_pages;
    PhaseContainer m_phases;
    PhaseNodeContainer m_phase_nodes;
    PopupPageDefContainer m_popupPageDefs;
    PopupPageContainer m_popupPages;
	ButtonAudioDefContainer m_buttonAudioDefs;
};

UICenter::~UICenter() {
}

::std::auto_ptr<UICenterExt>
UICenterExt::create(EnvExt & env, Cpe::Cfg::Node const & config) {
    return ::std::auto_ptr<UICenterExt>(new UICenterImpl(env, config));
}

}}

