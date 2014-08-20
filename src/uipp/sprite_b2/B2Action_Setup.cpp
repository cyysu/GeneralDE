#include <cassert>
#include "cpe/pal/pal_string.h"
#include "gdpp/app/Log.hpp"
#include "uipp/sprite_fsm/ActionReg.hpp"
#include "B2Action_Setup.hpp"
#include "B2WorldExt.hpp"
#include "B2ObjectPartExt.hpp"
#include "B2ObjectPartMeta.hpp"

namespace UI { namespace Sprite { namespace B2 {

B2Action_Setup::B2Action_Setup(Fsm::Action & action)
    : ActionBase(action)
{
}

B2Action_Setup::B2Action_Setup(Fsm::Action & action, B2Action_Setup const & o)
    : ActionBase(action)
    , m_setups(o.m_setups)
{
}

void B2Action_Setup::addSetup(AttrType type, float value, const char * part) {
    AttrSetupNode new_node;

    new_node.m_attrType = type;

    if (part) {
        strncpy(new_node.m_part, part, sizeof(new_node.m_part));
    }
    else {
        new_node.m_part[0] = 0;
    }

    new_node.m_new_value = value;
    new_node.m_saved_value = 0.0f;

    m_setups.push_back(new_node);
}

int B2Action_Setup::enter(void) {
    B2ObjectExt & b2Object = entity().component<B2ObjectExt>();		

    for(::std::vector<AttrSetupNode>::iterator it = m_setups.begin();
        it != m_setups.end();
        ++it)
    {
        it->m_saved_value = 
            (m_setup_funs[it->m_attrType])(b2Object, it->m_part, it->m_new_value);
    }

    return 0;
}

void B2Action_Setup::exit(void) {
    B2ObjectExt & b2Object = entity().component<B2ObjectExt>();		

    for(::std::vector<AttrSetupNode>::reverse_iterator it = m_setups.rbegin();
        it != m_setups.rend();
        ++it)
    {
        (m_setup_funs[it->m_attrType])(b2Object, it->m_part, it->m_saved_value);
    }
}

static float setupFixRotation(B2ObjectExt & b2Object, const char * part, float value) {
    uint8_t old_value = b2Object.fixedRotation();
    uint8_t new_value = value ? 1 : 0;

    if (old_value != new_value) {
        b2Object.setFixedRotation(new_value);
    }

    return old_value;
}

static float setupBullet(B2ObjectExt & b2Object, const char * part, float value) {
    uint8_t old_value = b2Object.bullet();
    uint8_t new_value = value ? 1 : 0;

    if (old_value != new_value) {
        b2Object.setBullet(new_value);
    }

    return old_value;
}

static float setupGravityScale(B2ObjectExt & b2Object, const char * part, float value) {
    float old_value = b2Object.gravityScale();

    if (old_value != value) {
        b2Object.setGravityScale(value);
    }

    return old_value;
}

static float setupMass(B2ObjectExt & b2Object, const char * part, float value) {
    float old_value = b2Object.mass();

    if (old_value != value) {
        b2Object.setMass(value);
    }

    return old_value;
}

static float setupFriction(B2ObjectExt & b2Object, const char * part_name, float value) {
    float old_value = 0.0f;
    bool have_old_value = false;

    B2ObjectExt::PartList & parts = b2Object.parts();
    for(B2ObjectExt::PartList::iterator it = parts.begin(); it != parts.end(); ++it) {
        B2ObjectPartExt & part = **it;

        if (strcmp(part_name, "*") != 0 && strcmp(part.meta().name().c_str(), part_name) != 0) continue;

        b2Fixture * b2Fixture = part.fixture();
        assert(b2Fixture);

        if (!have_old_value) {
            old_value = b2Fixture->GetFriction();
            have_old_value = true;
        }

        b2Fixture->SetFriction(value);
    }

    return old_value;
}

static float setupRestitution(B2ObjectExt & b2Object, const char * part_name, float value) {
    float old_value = 0.0f;
    bool have_old_value = false;

    B2ObjectExt::PartList & parts = b2Object.parts();
    for(B2ObjectExt::PartList::iterator it = parts.begin(); it != parts.end(); ++it) {
        B2ObjectPartExt & part = **it;

        if (strcmp(part_name, "*") != 0 && strcmp(part.meta().name().c_str(), part_name) != 0) continue;

        b2Fixture * b2Fixture = part.fixture();
        assert(b2Fixture);

        if (!have_old_value) {
            old_value = b2Fixture->GetRestitution();
            have_old_value = true;
        }

        b2Fixture->SetRestitution(value);
    }

    return old_value;
}

B2Action_Setup::setup_fun
B2Action_Setup::m_setup_funs[] = {
    setupFixRotation,
    setupBullet,
    setupGravityScale,
    setupMass,
    setupFriction,
    setupRestitution,
};

void B2Action_Setup::install(Fsm::Repository & repo) {
    Fsm::ActionReg<B2Action_Setup>(repo)
        .on_enter(&B2Action_Setup::enter)
        .on_exit(&B2Action_Setup::exit)
        ;
}

const char * B2Action_Setup::NAME = "b2-setup";

}}}

