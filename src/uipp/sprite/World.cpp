#include "gdpp/app/Application.hpp"
#include "gdpp/app/Log.hpp"
#include "uipp/sprite/World.hpp"
#include "uipp/sprite/Repository.hpp"

namespace UI { namespace Sprite {

Gd::App::Application & World::app(void) {
    return repository().app();
}

Gd::App::Application const & World::app(void) const {
    return repository().app();
}

WorldRes & World::res(const char * name) {
    WorldRes * e = findRes(name);

    if (e == NULL) {
        APP_CTX_THROW_EXCEPTION(repository().app(), ::std::runtime_error, "world res %s not exist!", name);
    }

    return *e;
}

WorldRes const & World::res(const char * name) const {
    WorldRes const * e = findRes(name);

    if (e == NULL) {
        APP_CTX_THROW_EXCEPTION(repository().app(), ::std::runtime_error, "world res %s not exist!", name);
    }

    return *e;
}

WorldRes & World::createRes(const char * name, size_t size) {
    ui_sprite_world_res_t res = ui_sprite_world_res_create(*this, name, size);

    if (res == NULL) {
        APP_CTX_THROW_EXCEPTION(repository().app(), ::std::runtime_error, "world create res %s(size=%d) fail!", name, (int)size);
    }

    return *(WorldRes*)res;
}

void World::removeRes(const char * name) {
    if (ui_sprite_world_res_t res = ui_sprite_world_res_find(*this, name)) {
        ui_sprite_world_res_free(res);
    }
}

Entity & World::entity(const char * name) {
    Entity * e = findEntity(name);

    if (e == NULL) {
        APP_CTX_THROW_EXCEPTION(repository().app(), ::std::runtime_error, "entity %s not exist!", name);
    }

    return *e;
}

Entity const & World::entity(const char * name) const {
    Entity const * e = findEntity(name);

    if (e == NULL) {
        APP_CTX_THROW_EXCEPTION(repository().app(), ::std::runtime_error, "entity %s not exist!", name);
    }

    return *e;
}

Entity & World::entity(uint32_t id) {
    Entity * e = findEntity(id);

    if (e == NULL) {
        APP_CTX_THROW_EXCEPTION(repository().app(), ::std::runtime_error, "entity %d not exist!", id);
    }

    return *e;
}

Entity const & World::entity(uint32_t id) const {
    Entity const * e = findEntity(id);

    if (e == NULL) {
        APP_CTX_THROW_EXCEPTION(repository().app(), ::std::runtime_error, "entity %d not exist!", id);
    }

    return *e;
}

Entity & World::createEntity(const char * name, const char * proto) {
    ui_sprite_entity_t entity = ui_sprite_entity_create(*this, name, proto);

    if (entity == NULL) {
        if (proto) {
            APP_CTX_THROW_EXCEPTION(
                repository().app(), ::std::runtime_error,
                "create entity %s(proto=%s) fail!", name, proto);
        }
        else {
            APP_CTX_THROW_EXCEPTION(repository().app(), ::std::runtime_error, "create entity %s fail!", name);
        }
    }

    return *(Entity*)entity;
}

Entity & World::proto(const char * name) {
    Entity * e = findProto(name);

    if (e == NULL) {
        APP_CTX_THROW_EXCEPTION(repository().app(), ::std::runtime_error, "proto %s not exist!", name);
    }

    return *e;
}

Entity const & World::proto(const char * name) const {
    Entity const * e = findProto(name);

    if (e == NULL) {
        APP_CTX_THROW_EXCEPTION(repository().app(), ::std::runtime_error, "proto %s not exist!", name);
    }

    return *e;
}

Entity & World::createProto(const char * name) {
    ui_sprite_entity_t entity = ui_sprite_entity_proto_create(*this, name);

    if (entity == NULL) {
        APP_CTX_THROW_EXCEPTION(
            repository().app(), ::std::runtime_error,
            "create proto %s fail!", name);
    }

    return *(Entity*)entity;
}

Group & World::group(const char * name) {
    Group * e = findGroup(name);

    if (e == NULL) {
        APP_CTX_THROW_EXCEPTION(repository().app(), ::std::runtime_error, "group %s not exist!", name);
    }

    return *e;
}

Group const & World::group(const char * name) const {
    Group const * e = findGroup(name);

    if (e == NULL) {
        APP_CTX_THROW_EXCEPTION(repository().app(), ::std::runtime_error, "group %s not exist!", name);
    }

    return *e;
}

Group & World::group(uint32_t id) {
    Group * e = findGroup(id);

    if (e == NULL) {
        APP_CTX_THROW_EXCEPTION(repository().app(), ::std::runtime_error, "group %d not exist!", id);
    }

    return *e;
}

Group const & World::group(uint32_t id) const {
    Group const * e = findGroup(id);

    if (e == NULL) {
        APP_CTX_THROW_EXCEPTION(repository().app(), ::std::runtime_error, "group %d not exist!", id);
    }

    return *e;
}

Group & World::createGroup(const char * name) {
    ui_sprite_group_t group = ui_sprite_group_create(*this, name);

    if (group == NULL) {
        APP_CTX_THROW_EXCEPTION(repository().app(), ::std::runtime_error, "create group %s fail!", name);
    }

    return *(Group*)group;
}

void World::addUpdator(ui_sprite_world_update_fun_t fun, void * ctx) {
    if (ui_sprite_world_add_updator(*this, fun, ctx) != 0) {
        APP_CTX_THROW_EXCEPTION(
            repository().app(), ::std::runtime_error,
            "world add updator fail!");
    }
}

World & World::create(Repository & repo) {
    ui_sprite_world_t world = ui_sprite_world_create(repo);

    if (world == NULL) {
        APP_CTX_THROW_EXCEPTION(repo.app(), ::std::runtime_error, "create world fail!");
    }

    return *(World *)world;
}


::std::auto_ptr<EntityIterator>
World::entities(mem_allocrator_t alloc) {
    ui_sprite_entity_it_t it = ui_sprite_world_entities(alloc, *this);
    return ::std::auto_ptr<EntityIterator>(new EntityIterator(it));
}

}}
