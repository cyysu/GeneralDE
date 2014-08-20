#ifndef UIPP_SPRITE_B2_SETUP_H
#define UIPP_SPRITE_B2_SETUP_H
#include <vector>
#include "cpepp/utils/ClassCategory.hpp"
#include "uipp/sprite_fsm/ActionGen.hpp"
#include "B2ObjectExt.hpp"

namespace UI { namespace Sprite { namespace B2 {

class B2ObjectPartExt;
class B2Action_Setup : public Fsm::ActionGen<Cpe::Utils::Noncopyable, B2Action_Setup> {
public:
    enum AttrType {
        AttrType_FixedRotation = 0,
        AttrType_Bullet,
        AttrType_GravityScale,
        AttrType_Mass,
        AttrType_Friction,
        AttrType_Restitution,
    };

    B2Action_Setup(Fsm::Action & action);
    B2Action_Setup(Fsm::Action & action, B2Action_Setup const & o);

    int enter(void);
    void exit(void);

    void addSetup(AttrType type, float value, const char * part = NULL);

    static const char * NAME;
    static void install(Fsm::Repository & repo);

private:
    struct AttrSetupNode {
        AttrType m_attrType;
        char m_part[32];
        float m_new_value;
        float m_saved_value;
    };

    ::std::vector<AttrSetupNode> m_setups;

    typedef float (*setup_fun)(B2ObjectExt & b2Object, const char * part, float value);

    static setup_fun m_setup_funs[];
};

}}}

#endif
