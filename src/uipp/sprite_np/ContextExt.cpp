#include <vector>
#include "NPRender.h"
#include "NP2DSTransRef.h"
#include "NP2DSActorRef.h"
#include "gdpp/app/Log.hpp"
#include "gdpp/app/Application.hpp"
#include "uipp/sprite/WorldResGen.hpp"
#include "uipp/sprite/WorldUpdatorGen.hpp"
#include "uipp/sprite_anim/AnimationCamera.hpp"
#include "uipp/sprite_anim/AnimationBackendGen.hpp"
#include "ContextExt.hpp"
#include "LayerExt.hpp"
#include "ModuleExt.hpp"
#include "AnimationFactory.hpp"
#include "NPControlNode.hpp"
#include "GroupNode.hpp"

namespace UI { namespace Sprite { namespace NP {

class ContextImpl
    : public WorldResGen<ContextExt, ContextImpl>
    , public WorldUpdatorGen<ContextImpl>
    , public Anim::AnimationBackendGen<ContextImpl>
{
public:
    ContextImpl(uipp_sprite_np_ext & module, WorldRes & world_res)
        : WorldResGen<ContextExt, ContextImpl>(world_res)
        , WorldUpdatorGen<ContextImpl>(world_res.world())
        , Anim::AnimationBackendGen<ContextImpl>(world_res.world(), this)
        , m_module(module)
        , m_default_layer(NULL)
        , m_maxId(1)
    {
        m_default_layer = &createLayer("default");

        if (Anim::AnimationCamera * camera = world_res.world().findRes<Anim::AnimationCamera>()) {
            onCameraUpdate(camera->pos(), camera->scalePair());
        }
        else {
            P2D::Pair pos = { 0.0f, 0.0f };
            P2D::Pair scale = { 1.0f, 1.0f };

            onCameraUpdate(pos, scale);
        }

        m_module.registerContext(*this);
    }

    ~ContextImpl() {

        m_module.unregisterContext(*this);

        for(::std::vector<LayerExt *>::iterator it = m_layers.begin();
            it != m_layers.end();
            ++it)
        {
            delete *it;
        }
        m_layers.clear();
        m_default_layer = NULL;
		clearExternCtrls();
    }

    virtual Layer & defaultLayer(void) {
        return * m_default_layer;
    }

    virtual Layer const & defaultLayer(void) const {
        return * m_default_layer;
    }

    virtual LayerExt * findLayer(const char * name) {
        if (name[0] == 0) return m_default_layer;

        for(::std::vector<LayerExt *>::iterator it = m_layers.begin();
            it != m_layers.end();
            ++it)
        {
            if (strcmp((*it)->name(), name) == 0) return *it;
        }

        return NULL;
    }

    virtual Layer const * findLayer(const char * name) const {
        if (name[0] == 0) return m_default_layer;

        for(::std::vector<LayerExt *>::const_iterator it = m_layers.begin();
            it != m_layers.end();
            ++it)
        {
            if (strcmp((*it)->name(), name) == 0) return *it;
        }

        return NULL;
    }

    virtual Layer & layer(const char * name) {
        Layer * l = findLayer(name);

        if (l == NULL) {
            APP_CTX_THROW_EXCEPTION(
                m_module.app(), ::std::runtime_error,
                "layer %s not exist", name);
        }

        return *l;
    }

    virtual Layer const & layer(const char * name) const {
        Layer const * l = findLayer(name);

        if (l == NULL) {
            APP_CTX_THROW_EXCEPTION(
                m_module.app(), ::std::runtime_error,
                "layer %s not exist", name);
        }

        return *l;
    }

    virtual LayerExt & createLayer(const char * name, Layer * before = NULL) {
        ::std::vector<LayerExt *>::iterator insert_pos = m_layers.end();

        for(::std::vector<LayerExt *>::iterator it = m_layers.begin();
            it != m_layers.end();
            ++it)
        {
            LayerExt * check_layer = *it;

            if (strcmp(check_layer->name(), name) == 0) {
                APP_CTX_THROW_EXCEPTION(
                    m_module.app(), ::std::runtime_error,
                    "layer %s already exist", name);
            }

            if (before == check_layer) insert_pos = it;
        }

        if (before && insert_pos == m_layers.end()) {
            APP_CTX_THROW_EXCEPTION(m_module.app(), ::std::runtime_error, "before layer unknown");
        }

        LayerExt * new_layer = new LayerExt(*this, name);
        m_layers.insert(insert_pos, new_layer);

        if (Anim::AnimationCamera * camera = world().findRes<Anim::AnimationCamera>()) {
            onCameraUpdate(camera->pos(), camera->scalePair());
        }

        return *new_layer;
    }

    struct CmpGroupPriority {
        bool operator() (NPRenderObject const * const l, NPRenderObject const * const r) {
            return 
                static_cast<GroupNode const *>(l)->m_priority
                < static_cast<GroupNode const *>(r)->m_priority;
        }
    };

    virtual void render(void) {
        for(::std::vector<LayerExt *>::reverse_iterator it = m_layers.rbegin();
            it != m_layers.rend();
            ++it)
        {
			LayerExt * layer = *it;

            if (layer->isDirty()) {
                layer->root().GetChildren().sort(CmpGroupPriority());
                layer->setIsDirty(false);
            }

			layer->root().Render();
        }
    }

    void onWorldUpdate(World & world, float delta) {
        for(::std::vector<LayerExt *>::iterator it = m_layers.begin();
            it != m_layers.end();
            ++it)
        {
            LayerExt * layer = *it;
			layer->root().Update(delta);
        }
    } 

    P2D::Pair screenSize(void) {
        P2D::Pair screen_size = { (float)NPRender::GetIns()->GetResolutionW(), (float)NPRender::GetIns()->GetResolutionH() };
        return screen_size;
    }

    uint32_t createGroup(const char * layer_name, uint32_t entityId, float priority) {
        LayerExt * layer = findLayer(layer_name);
        if (layer == NULL) {
			APP_CTX_ERROR(m_module.app(), "createGroup: layer %s not exist", layer_name);
            return UI_SPRITE_INVALID_ANIM_ID;
        }

        GroupNode * group = new GroupNode(world(), *layer, entityId, priority);

        layer->root().AddChild(group);

        uint32_t new_id = ++m_maxId;

		m_groups.insert(std::make_pair(new_id, group));

        layer->setIsDirty(true);

		return new_id;
    }

    void setGruopPriority(uint32_t groupId, float priority) {
        GroupMap::iterator pos = m_groups.find(groupId);
        if (pos == m_groups.end()) {
			APP_CTX_ERROR(m_module.app(), "setGruopPriority: group %d not exist", groupId);
            return;
        }

        GroupNode * g = pos->second;
        if (g->m_priority != priority) {
            g->m_priority = priority;
            g->m_layer.setIsDirty(true);
        }
    }

    void removeGroup(uint32_t groupId) {
        GroupMap::iterator pos = m_groups.find(groupId);
        if (pos == m_groups.end()) {
			APP_CTX_ERROR(m_module.app(), "removeGroup: group %d not exist", groupId);
            return;
        }

        NP2DSTransRef * group = pos->second;

        assert(group->GetChildCount() == 0);

        group->GetParent()->DelChild(group, true);
        
        m_groups.erase(pos);
    }

	uint32_t startAnimation(uint32_t groupId, const char * res, uint8_t is_loop, int32_t start, int32_t end) {
        GroupMap::iterator groupPos = m_groups.find(groupId);
        if (groupPos == m_groups.end()) {
			APP_CTX_ERROR(m_module.app(), "startAnimation %s: group %d not exist", res, groupId);
			return UI_SPRITE_INVALID_ANIM_ID;
        }

        NP2DSTransRef * group = groupPos->second;

        NPNode * anim =
            AnimationFactory::instance().createAnimation(
                m_module.app(), this, res, is_loop, start, end);
		if (anim == NULL) {
			APP_CTX_ERROR(m_module.app(), "startAnimation %s: create animation fail", res);
			return UI_SPRITE_INVALID_ANIM_ID;
        }

        group->AddChild(anim);

        uint32_t new_id = ++m_maxId;
		m_anims.insert(std::make_pair(new_id, anim));

		return new_id;
    }

    void stopAnimation(uint32_t anim_id) {
		AnimsMap::iterator pos = m_anims.find(anim_id);
		if(pos == m_anims.end()) {
			APP_ERROR("stopAnimation: Animation %d not exist", anim_id);
            return;
        }

        NPNode * node = pos->second;
        node->GetParent()->DelChild(node, true);
        m_anims.erase(pos);
    }

    bool isAnimationRuning(uint32_t anim_id) {
		AnimsMap::iterator iter= m_anims.find(anim_id);
		if(iter == m_anims.end()) return false;

        NPNode * anim = iter->second;
        if (NP2DSActorRef* actorRef = dynamic_cast<NP2DSActorRef*>(anim)) {
			return actorRef->WasPlay();
        }
		else if(NPControlNode* controlNode = dynamic_cast<NPControlNode*>(anim)){
			return controlNode->WasRuning();
		}
        else{
            return true;
        }
    }

    int setTemplateValue(uint32_t anim_id, const char * ctrl_name, const char * attr_name, const char * value) {
		AnimsMap::iterator iter= m_anims.find(anim_id);
		if(iter == m_anims.end()) {
			APP_CTX_ERROR(
                m_module.app(), "setTemplateValue: anim "FMT_UINT32_T" not exist", anim_id);
            return -1;
        }

        NPNode * anim = iter->second;
        NPControlNode * templateNode = dynamic_cast<NPControlNode *>(anim);
        if (templateNode == NULL) {
			APP_CTX_ERROR(
                m_module.app(), "setTemplateValue: anim "FMT_UINT32_T" is not NPControlNode", anim_id);
            return -1;
        }

        return templateNode->setAttr(ctrl_name, attr_name, value);
    }

	void onGroupPosUpdate(uint32_t group_id, P2D::Pair const & new_pos) {
		GroupMap::iterator pos = m_groups.find(group_id);
		if(pos == m_groups.end()) {
			APP_ERROR("onGroupPosUpdate: group %d not exist", group_id);
            return;
        }

        NP2DSTransRef * group = pos->second;
        group->SetWorldTrans(NPVector3(new_pos.x, new_pos.y, 0.0f));
	}

	void onGroupScaleUpdate(uint32_t group_id, P2D::Pair const & new_scale) {
		GroupMap::iterator pos = m_groups.find(group_id);
		if(pos == m_groups.end()) {
			APP_ERROR("onScaleUpdate: group %d not exist ", group_id);
            return;
        }

        NP2DSTransRef * group = pos->second;
        group->SetWorldScale(NPVector3(new_scale.x, new_scale.y, 0.0f));
	}

	void onGroupAngleUpdate(uint32_t group_id, float new_angle) {
		GroupMap::iterator pos = m_groups.find(group_id);
		if(pos == m_groups.end()) {
			APP_ERROR("onGroupAngleUpdate: group %d not exist ", group_id);
            return;
        }

        NP2DSTransRef * group = pos->second;
        group->SetWorldAngle(new_angle);
	}

	void onGroupFlipUpdate(uint32_t group_id, uint8_t flip_x, uint8_t flip_y) {
		GroupMap::iterator pos = m_groups.find(group_id);
		if(pos == m_groups.end()) {
			APP_ERROR("onGroupFlipUpdate: group %d not exist ", group_id);
            return;
        }

        NP2DSTransRef * group = pos->second;
        group->SetLocalFlips(flip_x | (flip_y << 1));
	}

	void onCameraUpdate(P2D::Pair const & pos, P2D::Pair const & scale) {
        for(::std::vector<LayerExt *>::iterator it = m_layers.begin();
            it != m_layers.end();
            ++it)
        {
            LayerExt * layer = *it;

            if (layer->isFree()) {
                layer->root().SetLocalS(NPVector3(1.0f, 1.0f, 1.0f));
                layer->root().SetLocalT(NPVector3(0.0f, 0.0f, 0.0f));
            }
            else {
                P2D::Pair adj_pos = pos;
                adj_pos.x *= layer->posAdj().x;
                adj_pos.y *= layer->posAdj().y;

                P2D::Pair adj_scale = scale;
                adj_scale.x += layer->scaleAdj().x;
                adj_scale.y += layer->scaleAdj().y;

                NPVector3 new_pos(- adj_pos.x * adj_scale.x, - adj_pos.y * adj_scale.y, 0.0f);
                NPVector3 new_scale(adj_scale.x, adj_scale.y, 0.0f);

                layer->root().SetLocalS(new_scale);
                layer->root().SetLocalT(new_pos);
            }
        }
	}

	virtual void registerExternCtrl(const char * name, NPGUIControl * ctrl) {
        assert(ctrl);
		m_controls.insert(ControlMap::value_type(name, ctrl));
	}

	virtual NPGUIControl * findExternCtrl(const char * name){
		ControlMap::iterator pos = m_controls.find(name);
        return pos == m_controls.end() ? NULL : pos->second;
	}

	virtual void unregisterExternCtrl(NPGUIControl * ctrl){
		for(ControlMap::iterator iter = m_controls.begin();
            iter!=m_controls.end();
            )
		{
            ControlMap::iterator next = iter;
            next++;

			if(iter->second == ctrl) {
				m_controls.erase(iter);
			}

            iter = next;

		}
	}

	virtual void clearExternCtrls(void){
        m_controls.clear();
	}

    virtual void setUpdatorPriority(int8_t priority) {
        WorldUpdatorGen<ContextImpl>::setUpdatorPriority(priority);
    }

private:
    typedef std::map< uint32_t, GroupNode * > GroupMap;
	typedef std::map< uint32_t, NPNode * > AnimsMap;
	typedef std::map< std::string, NPGUIControl *> ControlMap;

    uipp_sprite_np_ext & m_module;
    LayerExt * m_default_layer;
    ::std::vector<LayerExt *> m_layers;
    GroupMap m_groups;
	AnimsMap m_anims;
	ControlMap m_controls;
	uint32_t m_maxId;
};

Context::~Context() {
}

Context & Context::install(World & world) {
    return ContextImpl::install(
        dynamic_cast<uipp_sprite_np_ext &>(uipp_sprite_np::instance(world.app())),
        world);
}

const char * Context::NAME = "NPContext";

}}}

