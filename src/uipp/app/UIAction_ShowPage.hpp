#ifndef UIPP_APP_ACTION_SHOWPAGE_H
#define UIPP_APP_ACTION_SHOWPAGE_H
#include "cpepp/utils/ObjRef.hpp"
#include "cpepp/utils/ClassCategory.hpp"
#include "uipp/sprite_fsm/ActionGen.hpp"
#include "System.hpp"

namespace UI { namespace App {

class UIAction_ShowPage : public Sprite::Fsm::ActionGen<Cpe::Utils::Noncopyable, UIAction_ShowPage> {
public:
    UIAction_ShowPage(Sprite::Fsm::Action & action);
    UIAction_ShowPage(Sprite::Fsm::Action & action, UIAction_ShowPage const & o);

    int enter(void);
    void exit(void);
    void update(float delta);

    void setEnv(EnvExt & env) { m_env = env; }

    void setPageName(const char * page_name) { m_page_name = page_name; }
    const char * pageName(void) const { return m_page_name.c_str(); }

    static const char * NAME;
    static void install(Sprite::Fsm::Repository & repo);

private:
    Cpe::Utils::ObjRef<EnvExt> m_env;
    ::std::string m_page_name;
};

}}

#endif
