#include <string>
#include "uipp/sprite/World.hpp"
#include "LayerExt.hpp"
#include "ContextExt.hpp"

namespace UI { namespace Sprite { namespace NP {

LayerExt::LayerExt(ContextExt & ctx, const char * name)
    : m_name(name)
    , m_isDirty(false)
    , m_isFree(false)
{
    m_posAdj.x = 1.0f;
    m_posAdj.y = 1.0f;
    m_scaleAdj.x = 0.0f;
    m_scaleAdj.y = 0.0f;
}

LayerExt::~LayerExt() {
}

Layer::~Layer() {
}

}}}

