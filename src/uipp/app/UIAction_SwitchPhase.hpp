#ifndef UIPP_APP_ACTION_SWITCHPHASE_H
#define UIPP_APP_ACTION_SWITCHPHASE_H
#include "cpepp/utils/ObjRef.hpp"
#include "cpepp/utils/ClassCategory.hpp"
#include "uipp/sprite_fsm/ActionGen.hpp"
#include "System.hpp"

namespace UI { namespace App {

class UIAction_SwitchPhase : public Sprite::Fsm::ActionGen<Cpe::Utils::Noncopyable, UIAction_SwitchPhase> {
public:
    UIAction_SwitchPhase(Sprite::Fsm::Action & action);
    UIAction_SwitchPhase(Sprite::Fsm::Action & action, UIAction_SwitchPhase const & o);

    int enter(void);
    void exit(void);

    void setEnv(EnvExt & env) { m_env = env; }

    void setPhaseName(const char * phase_name) { m_phase_name = phase_name; }
    const char * phaseName(void) const { return m_phase_name.c_str(); }

    static const char * NAME;
    static void install(Sprite::Fsm::Repository & repo);

private:
    Cpe::Utils::ObjRef<EnvExt> m_env;
    ::std::string m_phase_name;
};

}}

#endif
