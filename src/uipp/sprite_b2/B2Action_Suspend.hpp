#ifndef UIPP_SPRITE_B2_SUSPEND_H
#define UIPP_SPRITE_B2_SUSPEND_H
#include "cpepp/utils/ClassCategory.hpp"
#include "uipp/sprite_fsm/ActionGen.hpp"
#include "B2ObjectExt.hpp"

namespace UI { namespace Sprite { namespace B2 {

class B2ObjectPartExt;
class B2Action_Suspend : public Fsm::ActionGen<Cpe::Utils::Noncopyable, B2Action_Suspend> {
public:
    B2Action_Suspend(Fsm::Action & action);
    B2Action_Suspend(Fsm::Action & action, B2Action_Suspend const & o);

    bool resume(void) const { return m_resume; }
    void setResume(bool resume) { m_resume = resume; }

    void setObjType(ObjectType objType) { m_change_to_type = objType; }
    void setRuningMode(RuningMode runing_mode) { m_change_to_runing_mode = runing_mode; }

    int enter(void);
    void exit(void);

    static const char * NAME;
    static void install(Fsm::Repository & repo);

private:
    bool m_resume;
    ObjectType m_change_to_type;
    RuningMode m_change_to_runing_mode;
};

}}}

#endif
