#ifndef UIPP_APP_ACTION_PLAYANIM_H
#define UIPP_APP_ACTION_PLAYANIM_H
#include "cpepp/utils/ObjRef.hpp"
#include "cpepp/utils/ClassCategory.hpp"
#include "uipp/sprite_fsm/ActionGen.hpp"
#include "System.hpp"

namespace UI { namespace App {

class UIAction_PlayAnim
    : public Sprite::Fsm::ActionGen<Cpe::Utils::Noncopyable, UIAction_PlayAnim>
{
public:
    UIAction_PlayAnim(Sprite::Fsm::Action & action);
    UIAction_PlayAnim(Sprite::Fsm::Action & action, UIAction_PlayAnim const & o);

    int enter(void);
    void exit(void);
    void update(float delta);

    void setEnv(EnvExt & env) { m_env = env; }

    void setPageName(const char * page_name) { m_page_name = page_name; }
    const char * pageName(void) const { return m_page_name.c_str(); }

    void setControlName(const char * control_name) { m_control_name = control_name; }
    const char * controlName(void) const { return m_control_name.c_str(); }

    void setAnimName(const char * anim_name) { m_anim_name = anim_name; }
    const char * animName(void) const { return m_anim_name.c_str(); }

    static const char * NAME;
    static void install(Sprite::Fsm::Repository & repo);

private:
    UIPageProxyExt * findPage(void);
    const NPGUIActorKeyData * getAnimData(NPGUIControl * control);

    Cpe::Utils::ObjRef<EnvExt> m_env;
    ::std::string m_page_name;
    ::std::string m_control_name;
    ::std::string m_anim_name;
};

}}

#endif
