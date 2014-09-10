#ifndef UIPP_APP_PAGECENTER_H
#define UIPP_APP_PAGECENTER_H
#include "cpepp/dr/Meta.hpp"
#include "System.hpp"

namespace UI { namespace App {

class UICenter {
public:
    virtual Env & env(void) = 0;
    virtual Env const & env(void) const = 0;

    virtual UIPageProxy & page(const char * name) = 0;
    virtual UIPageProxy const & page(const char * name) const = 0;

    virtual UIPageProxy * findPage(const char * name) = 0;
    virtual UIPageProxy const * findPage(const char * name) const = 0;

    virtual UIPhase * findPhase(const char * name) = 0;
    virtual UIPhase const * findPhase(const char * name) const = 0;

    virtual UIPhaseNode & curentPhase(void) = 0;
    virtual UIPhaseNode const & curentPhase(void) const = 0;

    virtual void phaseSwitch(const char * phase_name) = 0;
    virtual void phaseCall(const char * phase_name) = 0;
    virtual void phaseBack(void) = 0;

    virtual void sendEvent(LPDRMETA meta, void const * data, size_t data_size) = 0;

	virtual const char * findAudioByPostfix(const char * postfix) const = 0;

    template<typename T>
    void sendEvent(T const & data = T()) {
        sendEvent(Cpe::Dr::MetaTraits<T>::META, &data, Cpe::Dr::MetaTraits<T>::data_size(data));
    }

    /*PopupPages接口 */
    virtual void showPopupPage(const char * message, const char * template_name = NULL) = 0;
    virtual void showPopupPage(LPDRMETA meta, void const * data, size_t data_size, const char * template_name = NULL) = 0;
    virtual void showPopupErrorMsg(int error, const char * template_name = NULL) = 0;

    template<typename T>
    void showPopupPages(T const & data, const char * template_name = NULL) {
        showPopupPages(Cpe::Dr::MetaTraits<T>::META, &data, Cpe::Dr::MetaTraits<T>::data_size(data));
    }

    /*UI代理对象 */
    virtual Sprite::Entity & entity(void) = 0;
    virtual Sprite::Entity const & entity(void) const = 0;

    virtual ~UICenter();
};

}}

#endif
