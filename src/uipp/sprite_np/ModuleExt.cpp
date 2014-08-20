#include <vector>
#include "cpepp/nm/Manager.hpp"
#include "gdpp/app/Application.hpp"
#include "gdpp/app/Module.hpp"
#include "gdpp/app/ModuleDef.hpp"
#include "uipp/sprite/Repository.hpp"
#include "uipp/sprite_fsm/Repository.hpp"
#include "uipp/sprite/World.hpp"
#include "uipp/sprite/WorldRes.hpp"
#include "uipp/sprite_cfg/CfgLoaderExternGen.hpp"
#include "uipp/sprite_anim/AnimationBackend.hpp"
#include "uipp/sprite_np/Context.hpp"
#include "ModuleExt.hpp"
#include "ContextExt.hpp"
#include "LayerExt.hpp"

namespace UI { namespace Sprite { namespace NP {

class uipp_sprite_np_impl
    : public uipp_sprite_np_ext
    , public Cfg::CfgLoaderExternGen<uipp_sprite_np_impl>
{
public:
    uipp_sprite_np_impl(Gd::App::Application & app, Gd::App::Module & module, Cpe::Cfg::Node & cfg)
        : m_app(app)
    {
        addResourceLoader(&uipp_sprite_np_impl::initContext);
    }

    ~uipp_sprite_np_impl() {
        try {
            while(!m_contextes.empty()) {
                m_contextes.front()->destory();
            }
        }
        catch(...) {
        }
    }

    virtual Gd::App::Application & app(void)  {
        return m_app;
    }

    virtual Gd::App::Application const & app(void) const {
        return m_app;
    }

    virtual void registerContext(ContextExt & ctx) {
        m_contextes.push_back(&ctx);
    }

    virtual void unregisterContext(ContextExt & ctx) {
        for(::std::vector<ContextExt *>::iterator it = m_contextes.begin();
            it != m_contextes.end();
            ++it)
        {
            if (*it == &ctx) {
                m_contextes.erase(it);
                return;
            }
        }
    }

    virtual void render(void) {
        for(::std::vector<ContextExt *>::iterator it = m_contextes.begin();
            it != m_contextes.end();
            ++it)
        {
            (*it)->render();
        }
    }

private:
    void initContext(Context & context, Cpe::Cfg::Node const & cfg) const {
        Cpe::Cfg::NodeConstIterator layers = cfg["layers"].childs();
        Cpe::Cfg::Node const * defaultLayer = NULL;

        if (Cpe::Cfg::Node const * updatePriority = cfg.findChild("update-priority")) {
            context.setUpdatorPriority(updatePriority->asInt8());
        }

        while(Cpe::Cfg::Node const * check = layers.next()) {
            const char * layerName = check->isValue() ? check->asString() : check->onlyChild().name();
            if (strcmp(layerName, "default") == 0) {
                defaultLayer = check;
                break;
            }
        }
        
        layers = cfg["layers"].childs();

        if (defaultLayer) {
            LayerExt & dl = dynamic_cast<LayerExt&>(context.defaultLayer());

            while(Cpe::Cfg::Node const * node = layers.next()) {
                if (node == defaultLayer) break;

                createLayer(context, &dl, *node);
            }

            setupLayer(context, dl, *defaultLayer);

            while(Cpe::Cfg::Node const * node = layers.next()) {
                createLayer(context, NULL, *node);
            }
        }
        else {
            while(Cpe::Cfg::Node const * node = layers.next()) {
                createLayer(context, NULL, *node);
            }
        }
    }

    void createLayer(Context & context, LayerExt * before, Cpe::Cfg::Node const & node) const {
        const char * layerName = node.isValue() ? node.asString() : node.onlyChild().name();

        setupLayer(context, dynamic_cast<LayerExt&>(context.createLayer(layerName, before)), node);
    }

    void setupLayer(Context & context, LayerExt & layer, Cpe::Cfg::Node const & node) const {
        if (node.isValue()) return;

        if (node.onlyChild()["is-free"].asInt8(0)) {
            layer.setFree(true);
        }
        else {
            layer.setFree(false);

            Cpe::Cfg::Node const & scaleAdjNode = node.onlyChild()["scale-adj"];
            if (scaleAdjNode.isValid()) {
                P2D::Pair scaleAdj = layer.scaleAdj();
                scaleAdj.x = scaleAdjNode["x"].dft(scaleAdj.x);
                scaleAdj.y = scaleAdjNode["y"].dft(scaleAdj.y);
                layer.setScaleAdj(scaleAdj);
            }

            Cpe::Cfg::Node const & posAdjNode = node.onlyChild()["pos-adj"];
            if (posAdjNode.isValid()) {
                P2D::Pair posAdj = layer.posAdj();
                posAdj.x = posAdjNode["x"].dft(posAdj.x);
                posAdj.y = posAdjNode["y"].dft(posAdj.y);
                layer.setPosAdj(posAdj);
            }
        }
    }

    Gd::App::Application & m_app;
    ::std::vector<ContextExt *> m_contextes;
};

uipp_sprite_np_ext::~uipp_sprite_np_ext() {
}

uipp_sprite_np::~uipp_sprite_np() {
}

uipp_sprite_np & uipp_sprite_np::instance(Gd::App::Application & app) {
    uipp_sprite_np * r =
        dynamic_cast<uipp_sprite_np *>(
            &app.nmManager().object(NAME));
    if (r == NULL) {
        APP_THROW_EXCEPTION(::std::runtime_error, "uipp_sprite_np cast fail!");
    }
    return *r;
}

GDPP_APP_MODULE_DEF(uipp_sprite_np, uipp_sprite_np_impl);

}}}
