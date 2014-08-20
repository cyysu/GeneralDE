#include <vector>
#include "NPGUIDesktop.h"
#include "cpe/pal/pal_strings.h"
#include "cpepp/nm/Manager.hpp"
#include "cpepp/cfg/Node.hpp"
#include "cpe/dr/dr_data.h"
#include "gd/app/app_module.h"
#include "gdpp/app/Log.hpp"
#include "gdpp/app/Application.hpp"
#include "uipp/app/Page.hpp"
#include "UIPageProxyExt.hpp"
#include "UIPhaseNodeExt.hpp"
#include "UICenterExt.hpp"
#include "EnvExt.hpp"
#include "UIPageEventHandler.hpp"

namespace UI { namespace App {

class UIPageProxyImpl : public UIPageProxyExt {
public:
    UIPageProxyImpl(UICenterExt & center, Cpe::Cfg::Node const & config)
        : m_center(center)
        , m_page(createPage(center, config))
        , m_curentPhase(NULL)
    {
        m_page.m_proxy = this;

        try {
            installPage(config);
            m_page.SetVisible(false);
        }
        catch(...) {
            destoryPage();
            throw;
        }
    }

    ~UIPageProxyImpl() {
        assert(m_curentPhase == NULL);

        removePage();
        clearEventHandlers();
        destoryPage();
    }

    virtual const char * name(void) const {
        return m_page.name();
    }

    virtual UICenter & center(void) {
        return m_center;
    }

    virtual UICenter const & center(void) const {
        return m_center;
    }

    virtual Page & page(void) { 
        return m_page;
    }

    virtual Page const & page(void) const {
        return m_page;
    }

    virtual void addEventHandler(
        const char * event, Page::EventHandlerScope scope,
        ui_sprite_event_process_fun_t fun, void * ctx)
    {
        ::std::auto_ptr<UIPageEventHandler> handler(new UIPageEventHandler(event, scope, fun, ctx));
        m_eventHandlers.push_back(handler.get());
        handler.release();

        if (scope == Page::EventHandlerScopeAll
            || (scope == Page::EventHandlerScopeVisiable && m_page.WasVisible()))
        {
            m_eventHandlers.back()->active(entity());
        }
    }

    virtual void const * checkGetData(LPDRMETA meta) const {
        LPDRMETA pageMeta = m_page.pageDataMeta();
        if (pageMeta == NULL) {
            APP_CTX_THROW_EXCEPTION(
                m_page.app(), ::std::runtime_error,
                "page %s: get data: no data!", name());
        }

        if (pageMeta != meta) {
            APP_CTX_THROW_EXCEPTION(
                m_page.app(), ::std::runtime_error,
                "page %s: get data: meta is %s not %s!", name(), dr_meta_name(pageMeta), dr_meta_name(meta));
        }

        return m_page.pageData();
    }

    virtual void copyData(void * data, size_t data_capacity, LPDRMETA meta) const {
        bzero(data, data_capacity);

        LPDRMETA pageMeta = m_page.pageDataMeta();
        if (pageMeta == NULL) {
            APP_CTX_THROW_EXCEPTION(
                m_page.app(), ::std::runtime_error,
                "page %s: copy data: no data!", name());
        }

        if (dr_meta_copy_same_entry_part(
                data, data_capacity, meta,
                m_page.pageData(), m_page.pageDataSize(), m_page.pageDataMeta(),
                NULL, 0, m_page.app().em())
            < 0)
        {
            APP_CTX_THROW_EXCEPTION(
                m_page.app(), ::std::runtime_error,
                "page %s: copy data: page data %s, copy data to %s fail",
                name(), dr_meta_name(pageMeta), dr_meta_name(meta));
        }
    }

    virtual void setData(void const * data, size_t data_size, LPDRMETA meta) {
        LPDRMETA pageMeta = m_page.pageDataMeta();
        if (pageMeta == NULL) {
            APP_CTX_THROW_EXCEPTION(
                m_page.app(), ::std::runtime_error,
                "page %s: set data: no data!", name());
        }

        if (dr_meta_copy_same_entry_part(
                m_page.pageData(), m_page.pageDataSize(), m_page.pageDataMeta(),
                data, data_size, meta,
                NULL, 0, m_page.app().em())
            < 0)
        {
            APP_CTX_THROW_EXCEPTION(
                m_page.app(), ::std::runtime_error,
                "page %s: set data: page data %s, copy data from %s fail",
                name(), dr_meta_name(pageMeta), dr_meta_name(meta));
        }
    }

    virtual void onVisiableUpdate() {
        if (m_page.WasVisible()) {
            if (m_curentPhase) {
                UIPhaseNodeExt & curentPhase = 
                    dynamic_cast<EnvExt&>(m_page.env()).uiCenter().curentPhase();
                if (&curentPhase != m_curentPhase) {
                    m_curentPhase->removeVisiablePage(*this);
                    m_curentPhase = &curentPhase;
                    m_curentPhase->addVisiablePage(*this);
                }
            }
            
            activeVisiableEventHandlers();
        }
        else {
            if (m_curentPhase) {
                m_curentPhase->removeVisiablePage(*this);
                m_curentPhase = NULL;
            }

            deactiveVisiableEventHandlers();
        }
    }

private:
    static Page & createPage(UICenterExt & center, Cpe::Cfg::Node const & config) {
        const char * page_name = config.name();
        const char * page_type = config["type"].dft(page_name);

        gd_app_module_t module = gd_app_install_module(center.env().app(), page_name, page_type, NULL, config);
        if (module == NULL) {
            APP_CTX_THROW_EXCEPTION(
                center.env().app(), ::std::runtime_error,
                "load page %s type %s fail!",
                page_name, page_type);
        }

        return dynamic_cast<Page&>(center.env().app().nmManager().objectNc(page_name));
    }

    void clearEventHandlers(void) {
        Sprite::Entity & e = entity();

        for(::std::vector<UIPageEventHandler *>::iterator it = m_eventHandlers.begin();
            it != m_eventHandlers.end();
            ++it)
        {
            UIPageEventHandler * handler = *it;
            if (handler->isActive()) handler->deactive(e);
            delete handler;
        }

        m_eventHandlers.clear();
    }

    void activeVisiableEventHandlers(void) {
        Sprite::Entity & e = entity();

        for(::std::vector<UIPageEventHandler *>::iterator it = m_eventHandlers.begin();
            it != m_eventHandlers.end();
            ++it)
        {
            UIPageEventHandler * handler = *it;

            if (handler->scope() != Page::EventHandlerScopeVisiable) continue;
            if (handler->isActive()) continue;

            handler->active(e);
        }
    }

    void deactiveVisiableEventHandlers(void) {
        Sprite::Entity & e = entity();

        for(::std::vector<UIPageEventHandler *>::iterator it = m_eventHandlers.begin();
            it != m_eventHandlers.end();
            ++it)
        {
            UIPageEventHandler * handler = *it;

            if (handler->scope() != Page::EventHandlerScopeVisiable) continue;
            if (!handler->isActive()) continue;

            handler->deactive(e);
        }
    }

    void destoryPage(void) {
        m_page.m_proxy = NULL;
        gd_app_uninstall_module(m_page.env().app(), name());
    }

    void installPage(Cpe::Cfg::Node const & cfg) {
        const char * load_from = cfg["load-from"].asString(NULL);
        if (load_from == NULL) {
            APP_CTX_THROW_EXCEPTION(
                m_page.env().app(), ::std::runtime_error, "load page %s: load-from not configured!", name());
        }

        if (!m_page.LoadBinFile(load_from)) {
            APP_CTX_THROW_EXCEPTION(
                m_page.env().app(), ::std::runtime_error, "load page %s: load-from %s fail!", name(), load_from);
        }

        NPGUIDesktop::GetIns()->AddWindow(&m_page);
    }

    void removePage() {
        NPGUIDesktop::GetIns()->DelChild(&m_page, false);
    }

    Sprite::Entity & entity(void) {
        return m_center.entity();
    }

    UICenterExt & m_center;
    Page & m_page;
    UIPhaseNodeExt * m_curentPhase;
    ::std::vector<UIPageEventHandler *> m_eventHandlers;
};

UIPageProxy::~UIPageProxy() {
}

::std::auto_ptr<UIPageProxyExt>
UIPageProxyExt::create(UICenterExt & center, Cpe::Cfg::Node const & config) {
    return ::std::auto_ptr<UIPageProxyExt>(new UIPageProxyImpl(center, config));
}

}}

