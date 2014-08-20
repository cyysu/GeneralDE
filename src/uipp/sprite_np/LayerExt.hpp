#ifndef UIPP_SPRITE_NP_LAYER_EXT_H
#define UIPP_SPRITE_NP_LAYER_EXT_H
#include <memory>
#include "uipp/sprite_np/Layer.hpp"
#include "uipp/sprite_2d/System.hpp"
#include "NPNode.h"

namespace UI { namespace Sprite { namespace NP {

class ContextExt;
class LayerExt : public Layer {
public:
    LayerExt(ContextExt & ctx, const char * name);
    ~LayerExt();

    virtual const char * name(void) const { return m_name.c_str(); }

    NPNode & root(void) { return m_root; }
    NPNode const & root(void) const { return m_root; }

    P2D::Pair const & posAdj(void) const { return m_posAdj; }
    void setPosAdj(P2D::Pair const & pos) { m_posAdj = pos; }

    P2D::Pair const & scaleAdj(void) const { return m_scaleAdj; }
    void setScaleAdj(P2D::Pair const & scale) { m_scaleAdj = scale; }

    bool isFree(void) const { return m_isFree; }
    void setFree(bool f) { m_isFree = f; }

    bool isDirty(void) const { return m_isDirty; }
    void setIsDirty(bool isDirty) { m_isDirty = isDirty; }

private:
    ::std::string m_name;
    NPNode m_root;
    bool m_isDirty;

    P2D::Pair m_posAdj;
    P2D::Pair m_scaleAdj;
    bool m_isFree;
};

}}}

#endif
