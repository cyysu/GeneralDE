#include <cassert>
#include <vector>
#include "cpepp/utils/ObjHolder.hpp"
#include "cpepp/cfg/Tree.hpp"
#include "uipp/sprite/Entity.hpp"
#include "uipp/sprite/Component.hpp"
#include "uipp/sprite_fsm/Fsm.hpp"
#include "uipp/sprite_fsm/State.hpp"
#include "UIPhaseNodeExt.hpp"
#include "UICenterExt.hpp"

namespace UI { namespace App {

class UIPhaseNodeImpl : public UIPhaseNodeExt {
public:
    UIPhaseNodeImpl(UIPhaseExt & phase)
        : m_phase(phase)
    {
    }

    virtual UIPhaseExt const & phase(void) const {
        return m_phase;
    }

    virtual void addVisiablePage(UIPageProxyExt & page) {
        for(::std::vector<UIPageProxyExt *>::iterator it = m_visiablePages.begin();
            it != m_visiablePages.end();
            ++it)
        {
            if (*it == &page) {
                assert(false);
            }
        }

        m_visiablePages.push_back(&page);
    }

    virtual void removeVisiablePage(UIPageProxyExt & page) {
        for(::std::vector<UIPageProxyExt *>::iterator it = m_visiablePages.begin();
            it != m_visiablePages.end();
            ++it)
        {
            if (*it == &page) {
                m_visiablePages.erase(it);
                return;
            }
        }

        assert(false);
    }

    virtual size_t visiablePageCount(void) const {
        return m_visiablePages.size();
    }

    virtual UIPageProxyExt const & visiablePageAt(size_t pos) const {
        assert(pos < m_visiablePages.size());
        return *m_visiablePages[pos];
    }

    virtual void saveState(void) {
        Sprite::Entity & entity = m_phase.center().entity();
        
        Sprite::Fsm::ComponentFsm & fsm = entity.component<Sprite::Fsm::ComponentFsm>();
        Sprite::Fsm::State * curentState = fsm.currentState();
        Sprite::Fsm::State * callState = NULL;

        assert(curentState);
        if (curentState->returnTo()) {
            callState = curentState;
            curentState = curentState->returnTo();
        }

        ::std::auto_ptr<Cpe::Cfg::Tree> savedData(new Cpe::Cfg::Tree());

        Cpe::Cfg::Node & data = savedData->root();
        data["init-state"] = curentState->name();
        if (callState) {
            data["init-call-state"] = callState->name();
        }

        
        m_savedData.reset(savedData);
    }

    virtual bool enter(void) {
        Sprite::Entity & entity = m_phase.center().entity();
        assert(m_visiablePages.empty());

        try {
            Sprite::Fsm::ComponentFsm & fsm = entity.createComponent<Sprite::Fsm::ComponentFsm>();
            fsm.copy(m_phase.runingFsm());

            if (m_savedData.valid()) {
                Cpe::Cfg::Node const & data = m_savedData.get().root();

                if (const char * init_state_name = data["init-state"].asString(NULL)) {
                    if (fsm.findState(init_state_name)) {
                        fsm.setDefaultState(init_state_name);
                    }
                }

                if (const char * init_call_name = data["init-call-state"].asString(NULL)) {
                    if (fsm.findState(init_call_name)) {
                        fsm.setDefaultCallState(init_call_name);
                    }
                }
            }

            fsm.component().enter();
            return true;
        }
        catch(...) {
            entity.removeComponent<Sprite::Fsm::ComponentFsm>();
            return false;
        }
    }

    virtual void exit(void) {
        Sprite::Entity & entity = m_phase.center().entity();

        try {
            entity.removeComponent<Sprite::Fsm::ComponentFsm>();
        }
        catch(...) {
        }

        assert(m_visiablePages.empty());
    }

private:
    UIPhaseExt & m_phase;
    ::std::vector<UIPageProxyExt *> m_visiablePages;
    Cpe::Utils::ObjHolder<Cpe::Cfg::Tree> m_savedData;
};

UIPhaseNode::~UIPhaseNode() {
}

::std::auto_ptr<UIPhaseNodeExt>
UIPhaseNodeExt::create(UIPhaseExt & phase) {
    return ::std::auto_ptr<UIPhaseNodeExt>(new UIPhaseNodeImpl(phase));
}

}}
