#ifndef UIPP_APP_UIPHASE_EXT_H
#define UIPP_APP_UIPHASE_EXT_H
#include <set>
#include "uipp/app/UIPhase.hpp"
#include "System.hpp"

namespace UI { namespace App {

class UIPhaseExt : public UIPhase {
public:
    virtual UICenterExt & center(void) = 0;
    virtual UICenterExt const & center(void) const = 0;

    virtual Sprite::Fsm::Fsm const & runingFsm(void) const = 0;
    virtual ::std::set< ::std::string> const & usingTextures(void) const = 0;
	virtual ::std::set< ::std::string> const & usingSFX(void) const = 0;
	virtual ::std::set< ::std::string> const & playBGM(void) const = 0;

    static ::std::auto_ptr<UIPhaseExt> create(UICenterExt & center, Cpe::Cfg::Node const & config);
};

}}

#endif


