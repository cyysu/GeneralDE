#ifndef UIPP_SPRITE_WORLD_H
#define UIPP_SPRITE_WORLD_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/dr/Utils.hpp"
#include <memory>
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_group.h"
#include "WorldRes.hpp"
#include "EntityIterator.hpp"

namespace UI { namespace Sprite {

class World : public Cpe::Utils::SimulateObject {
public:
    operator ui_sprite_world_t () const { return (ui_sprite_world_t)this; }

    Repository & repository(void) { return *(Repository*)ui_sprite_world_repository(*this); }
    Repository const & repository(void) const { return *(Repository const *)ui_sprite_world_repository(*this); }

    Gd::App::Application & app(void);
    Gd::App::Application const & app(void) const;

    void destory(void) { ui_sprite_world_free(*this); }

    float fps(void) const { return ui_sprite_world_fps(*this); }
    void setFps(float fps) { ui_sprite_world_set_fps(*this,  fps); }

	float tickAdj(void) const { return ui_sprite_world_tick_adj(*this); }
	void setTickAdj(float adj) { ui_sprite_world_set_tick_adj(*this,  adj); }

    ::std::auto_ptr<EntityIterator> entities(mem_allocrator_t alloc = NULL);

    /*res operations*/
    WorldRes * findRes(const char * name) { return (WorldRes*)ui_sprite_world_res_find(*this, name); }
    WorldRes const * findRes(const char * name) const { return (WorldRes const *)ui_sprite_world_res_find(*this, name); }

    WorldRes & res(const char * name);
    WorldRes const & res(const char * name) const;

    WorldRes & createRes(const char * name, size_t size);

    void removeRes(const char * name);

    template<typename T>
    T & res(void) { return *(T*)res(T::NAME).data(); } 

    template<typename T>
    T const & res(void) const { return *(T const *)res(T::NAME).data(); } 

    template<typename T>
    T * findRes(void) { WorldRes * r = findRes(T::NAME); return r ? (T*)r->data() : NULL; } 

    template<typename T>
    T const * findRes(void) const { WorldRes const * r = findRes(T::NAME); return r ? (T const *)r->data() : NULL; } 

    template<typename T>
    void removeRes(void) { removeRes(T::NAME); }

    template<typename T>
    T & createRes(void) { return T::install(*this); }

    /*updator*/
    void addUpdator(ui_sprite_world_update_fun_t fun, void * ctx);
    void removeUpdator(void * ctx) { ui_sprite_world_remove_updator(*this, ctx); }
    void setUpdatorPriority(void * ctx, int8_t priority);

    /*entity operations*/
    Entity * findEntity(const char * name) { return (Entity*)ui_sprite_entity_find_by_name(*this, name); }
    Entity const * findEntity(const char * name) const { return (Entity*)ui_sprite_entity_find_by_name(*this, name); }
    Entity & entity(const char * name);
    Entity const & entity(const char * name) const;

    Entity * findEntity(uint32_t id) { return (Entity*)ui_sprite_entity_find_by_id(*this, id); }
    Entity const * findEntity(uint32_t id) const { return (Entity*)ui_sprite_entity_find_by_id(*this, id); }
    Entity & entity(uint32_t id);
    Entity const & entity(uint32_t id) const;

    Entity & createEntity(const char * name, const char * proto = NULL);

    /*proto operations*/
    Entity * findProto(const char * name) { return (Entity*)ui_sprite_entity_proto_find(*this, name); }
    Entity const * findProto(const char * name) const { return (Entity*)ui_sprite_entity_proto_find(*this, name); }
    Entity & proto(const char * name);
    Entity const & proto(const char * name) const;

    Entity & createProto(const char * name);
    void removeProto(const char * name);

    /*group operations*/
    Group * findGroup(const char * name) { return (Group*)ui_sprite_group_find_by_name(*this, name); }
    Group const * findGroup(const char * name) const { return (Group*)ui_sprite_group_find_by_name(*this, name); }
    Group & group(const char * name);
    Group const & group(const char * name) const;

    Group * findGroup(uint32_t id) { return (Group*)ui_sprite_group_find_by_id(*this, id); }
    Group const * findGroup(uint32_t id) const { return (Group*)ui_sprite_group_find_by_id(*this, id); }
    Group & group(uint32_t id);
    Group const & group(uint32_t id) const;

    Group & createGroup(const char * name);

    /*event operations*/
    void sendEvent(LPDRMETA meta, void const * data, size_t data_size) { ui_sprite_world_send_event(*this, meta, data, data_size); }
    void sendEvent(const char * entity, LPDRMETA meta, void const * data, size_t data_size) { ui_sprite_world_send_event_to(*this, entity, meta, data, data_size); }

    template<typename T>
    void sendEvent(T const & data) {
        ui_sprite_world_send_event(
            *this,
            Cpe::Dr::MetaTraits<T>::META, &data, Cpe::Dr::MetaTraits<T>::data_size(data));
    }

    template<typename T>
    void sendEventTo(const char * targets, T const & data) {
        ui_sprite_world_send_event_to(
            *this, targets,
            Cpe::Dr::MetaTraits<T>::META, &data, Cpe::Dr::MetaTraits<T>::data_size(data));
    }

    static World & create(Repository & repo);
};

}}

#endif