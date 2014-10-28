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
#include "uipp/sprite_render/Context.hpp"
#include "ModuleExt.hpp"
#include "ContextExt.hpp"
#include "AnimationFactory.hpp"
#include "LayerExt.hpp"

namespace UI { namespace Sprite { namespace R {

class uipp_sprite_render_impl
    : public uipp_sprite_render_ext
    , public Cfg::CfgLoaderExternGen<uipp_sprite_render_impl>
{
public:
    uipp_sprite_render_impl(Gd::App::Application & app, Gd::App::Module & module, Cpe::Cfg::Node & cfg)
        : m_app(app)
    {
        addResourceLoader(&uipp_sprite_render_impl::initContext);
    }

    ~uipp_sprite_render_impl() {
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

    virtual node_builder const * findNodeBuilder(const char * name, size_t name_len) const {
        for(::std::vector<node_builder>::const_iterator it = m_node_builders.begin();
            it != m_node_builders.end();
            ++it)
        {
            if (cpe_str_cmp_part(name, name_len, it->m_name) == 0) return &*it;
        }

        return NULL;
    }

    virtual void registerRender(const char * name, node_build_fun_t build_fun, void * build_ctx) {
        m_node_builders.push_back(node_builder());

        node_builder & builder = m_node_builders.back();
        strncpy(builder.m_name, name, sizeof(builder.m_name));
        builder.m_build_fun = build_fun;
        builder.m_build_ctx = build_ctx;
    }

    virtual void unregisterRender(const char * name) {
        for(::std::vector<node_builder>::iterator it = m_node_builders.begin();
            it != m_node_builders.end();
            ++it)
        {
            if (strcmp(it->m_name, name) == 0) {
                m_node_builders.erase(it);
                return;
            }
        }
    }

private:
    void initContext(Context & context_i, Cpe::Cfg::Node const & cfg) const {
        ContextExt & context = dynamic_cast<ContextExt&>(context_i);
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

    void createLayer(ContextExt & context, LayerExt * before, Cpe::Cfg::Node const & node) const {
        const char * layerName = node.isValue() ? node.asString() : node.onlyChild().name();

        setupLayer(context, dynamic_cast<LayerExt&>(context.createLayer(layerName, before)), node);
    }

    void setupLayer(ContextExt & context, LayerExt & layer, Cpe::Cfg::Node const & node) const {
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

        Cpe::Cfg::NodeConstIterator initNodes = node.onlyChild()["init"].childs();
        while(Cpe::Cfg::Node const * initNode = initNodes.next()) {
            const char * initNodeUrl = initNode->asString(NULL);
            if (initNodeUrl == NULL) {
                APP_CTX_THROW_EXCEPTION(m_app, ::std::runtime_error, "layer init node config format error!");
            }

            RNode * anim = AnimationFactory::instance().createAnimation(m_app, &context, initNodeUrl, 1, -1, -1);
            if (anim == NULL) {
                APP_CTX_THROW_EXCEPTION(m_app, ::std::runtime_error, "layer init: create animation %s fail", initNodeUrl);
            }

            layer.root().AddChild(anim);
        }
    }

    Gd::App::Application & m_app;
    ::std::vector<ContextExt *> m_contextes;
    ::std::vector<node_builder> m_node_builders;
};

uipp_sprite_render_ext::~uipp_sprite_render_ext() {
}

uipp_sprite_render::~uipp_sprite_render() {
}

uipp_sprite_render & uipp_sprite_render::instance(Gd::App::Application & app) {
    uipp_sprite_render * r =
        dynamic_cast<uipp_sprite_render *>(
            &app.nmManager().object(NAME));
    if (r == NULL) {
        APP_THROW_EXCEPTION(::std::runtime_error, "uipp_sprite_render cast fail!");
    }
    return *r;
}

GDPP_APP_MODULE_DEF(uipp_sprite_render, uipp_sprite_render_impl);

}}}
