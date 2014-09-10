#ifndef UIPP_APP_ACTION_SETUP_CONTROL_H
#define UIPP_APP_ACTION_SETUP_CONTROL_H
#include "cpepp/utils/ObjRef.hpp"
#include "cpepp/utils/ClassCategory.hpp"
#include "ui/utils/ui_percent_decorator.h"
#include "uipp/sprite_fsm/ActionGen.hpp"
#include "uipp/sprite_2d/System.hpp"
#include "System.hpp"

namespace UI { namespace App {

class UIAction_SetupControl : public Sprite::Fsm::ActionGen<Cpe::Utils::Noncopyable, UIAction_SetupControl> {
public:
    UIAction_SetupControl(Sprite::Fsm::Action & action);
    UIAction_SetupControl(Sprite::Fsm::Action & action, UIAction_SetupControl const & o);

    int enter(void);
    void exit(void);

    void setEnv(EnvExt & env) { m_env = env; }

	void setPageName(const char * page_name) { m_page_name = page_name; }
	const char * pageName(void) const { return m_page_name.c_str(); }

	void setControlName(const char * cotrol_name) { m_cotrol_name = cotrol_name; }
	const char * cotrolName(void) const { return m_cotrol_name.c_str(); }

    void setRestore(uint8_t restore) { m_cfg_restore = restore; }
    uint8_t restore(void) const { return m_cfg_restore; }

    void setVisiable(uint8_t visiable) { m_cfg_visiable = visiable; }
    uint8_t visiable(void) const { return m_cfg_visiable; }

    static const char * NAME;
    static void install(Sprite::Fsm::Repository & repo);

private:
	Cpe::Utils::ObjRef<EnvExt> m_env;
	::std::string m_page_name;
	::std::string m_cotrol_name;
    uint8_t m_cfg_restore;
    uint8_t m_cfg_visiable;

    uint8_t m_saved_visiable;
};

}}

#endif
