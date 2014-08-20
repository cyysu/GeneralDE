#ifndef UIPP_APP_PAGE_H
#define UIPP_APP_PAGE_H
#include <sstream>
#include "cpepp/dr/Meta.hpp"
#include "NPGUIWindow.h"
#include "cpepp/nm/Object.hpp"
#include "gdpp/app/Application.hpp"
#include "System.hpp"

namespace UI { namespace App {

class UIPageProxyImpl;
class Page
    : public Cpe::Nm::Object
    , public NPGUIWindow
{
public:
    enum EventHandlerScope {
        EventHandlerScopeAll,
        EventHandlerScopeVisiable
    };

    Page(Gd::App::Application & app, Cpe::Cfg::Node & cfg);
    virtual ~Page();

    Gd::App::Application & app(void);
    Gd::App::Application const & app(void) const;

    Env & env(void);
    Env const & env(void) const;

    UICenter & uiCenter(void);
    UICenter const & uiCenter(void) const;

    void sendEvent(LPDRMETA meta, void const * data, size_t data_size);

    template<typename T>
    void sendEvent(T const & data = T()) {
        sendEvent(Cpe::Dr::MetaTraits<T>::META, &data, Cpe::Dr::MetaTraits<T>::data_size(data));
    }

    virtual void SetVisible(bool flag);

    void setControlEnable(NPGUIControl * control, const char * name, bool is_enable);
    void setControlEnable(const char * name, bool is_enable) { setControlEnable(this, name, is_enable); } 

    void setControlVisible(NPGUIControl * control, const char * name, bool is_visiable);
    void setControlVisible(const char * name, bool is_visiable) { setControlVisible(this, name, is_visiable); }

    void setLabelText(NPGUIControl * control, const char * name, const char * text);
    void setLabelText(const char * name, const char * text) { setLabelText(this, name, text); }

    void setLabelText(NPGUIControl * control, const char * name, int value);
    void setLabelText(const char * name, int text) { setLabelText(this, name, text); }

    void setIndex(NPGUIControl * control, const char * name, int index);
    void setIndex(const char * name, int index) { setIndex(this, name, index); }

    void setListCount(NPGUIControl * control, const char * name, uint32_t item_count);
    void setListCount(const char * name, uint32_t item_count) { setListCount(this, name, item_count); }

    void setProgress(NPGUIControl * control, const char * name, float percent);
    void setProgress(const char * name, float percent) { setProgress(this, name, percent); }

    void setUserData(NPGUIControl * control, const char * data);
    void setUserData(NPGUIControl * control, const char * name, const char * data);

    void setShowAnimPlay(NPGUIControl * control, const char * name);
    void setShowAnimPlay(const char * name) { setShowAnimPlay(this, name); }
    void setShowAnimPlay(NPGUIControl * control);

    void setHideAnimPlay(NPGUIControl * control, const char * name);
    void setHideAnimPlay(const char * name) { setHideAnimPlay(this, name); }
    void setHideAnimPlay(NPGUIControl * control);

	void setBackFrame(NPGUIControl * control, const char * name, const char * resource);
	void setBackFrame(const char * name, const char * resource) { setBackFrame(this, name, resource); }

	bool isChildOf(NPGUIControl* control, const char * name) const;

	bool isControlNameWith(NPGUIControl* control, const char * str) const;

    static NPGUIControl * findChild(NPGUIControl * control, const char * name, UICenter const & uiCenter);
    NPGUIControl * findChild(NPGUIControl * control, const char * name);
    NPGUIControl * findChild(const char * name) { return findChild(this, name); }

    static bool isControlNameEq(NPGUIControl * control, const char * name, UICenter const & uiCenter);
    bool isControlNameEq(NPGUIControl * control, const char * name) const;

    ::std::string getText(NPGUIControl * control, const char * name);
    ::std::string getText(const char * name) { return getText(this, name); }

    bool haveTrigger(NPGUIControl const * control) const;

    template<typename T>
    T getUserData(NPGUIControl * control) {
        ::std::istringstream is(control->GetUserText());
        T v;
        is >> v;
        return v;
    }

    template<typename T>
    T getUserData(NPGUIControl * control, const char * name) {
        NPGUIControl * c = findChild(control, name);
        assert(c);
        return getUserData<T>(c);
    }

    template<typename T>
    void setUserData(NPGUIControl * control, T const & data) {
        ::std::ostringstream os;
        os << data;
        setUserData(control, os.str().c_str());
    }

    template<typename T>
    void setUserData(NPGUIControl * control, const char * name, T const & data) {
        ::std::ostringstream os;
        os << data;
        setUserData(control, name, os.str().c_str());
    }

    PageEvtSch & schedule(void) { return *m_schedule; }
    PageEvtSch const & schedule(void) const { return *m_schedule; }

    template<typename T>
    void setPageData(T & data) {
        setPageData(Cpe::Dr::MetaTraits<T>::META, &data, Cpe::Dr::MetaTraits<T>::data_size(data));
    }

    void setPageData(LPDRMETA meta, void * data, uint32_t data_size) {
        m_data_meta = meta;
        m_data = data;
        m_data_size = data_size;
    }

    LPDRMETA pageDataMeta(void) const { return m_data_meta; }
    void * pageData(void) { return m_data; }
    void const * pageData(void) const { return m_data; }
    uint32_t pageDataSize(void) const { return m_data_size; }

    void addEventHandler(const char * event, EventHandlerScope scope, ui_sprite_event_process_fun_t fun, void * ctx);

    /*PopupPages接口 */
    void showPopupPage(const char * message, const char * template_name = NULL);
    void showPopupPage(LPDRMETA meta, void const * data, size_t data_size, const char * template_name = NULL);
    void showPopupErrorMsg(int error, const char * template_name = NULL);

    template<typename T>
    void showPopupPages(T const & data, const char * template_name = NULL) {
        showPopupPages(Cpe::Dr::MetaTraits<T>::META, &data, Cpe::Dr::MetaTraits<T>::data_size(data));
    }

protected:
    void OnShow(NPGUIEventArgs& args);
    void OnEventMouseClick(NPGUIEventArgs& args);
	void OnEventMouseDown(NPGUIEventArgs& args);
    void OnEventListBoxItemShow(NPGUIEventArgs& args);
	bool readResId(char * res, int & id);

private:
    friend class UIPageProxyImpl;

    Gd::App::Application & m_app;
    PageEvtSch * m_schedule;
    UIPageProxy * m_proxy;
    LPDRMETA m_data_meta;
    void * m_data;
    uint32_t m_data_size;
};

}}

#endif
