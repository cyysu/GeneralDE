#ifndef UIPP_SPRITE_B2_OBJECT_PART_H
#define UIPP_SPRITE_B2_OBJECT_PART_H
#include "cpepp/utils/ClassCategory.hpp"
#include "B2System.hpp"

namespace UI { namespace Sprite { namespace B2 {

class B2ObjectPart : public Cpe::Utils::Noncopyable {
public:
    void createShape(UI_SPRITE_B2_SHAPE const & shap);

    ~B2ObjectPart();
};

}}}

#endif

