#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_data_entry.h"
#include "ui_sprite_data_build_i.h"
#include "ui_sprite_entity_i.h"

static int ui_sprite_data_build_set_entity_id(dr_data_entry_t to, ui_sprite_repository_t repo, ui_sprite_entity_t entity) {
    if (entity == NULL) {
        CPE_ERROR(
            repo->m_em, "set %s(%s) from entity-id: no source entity!",
            dr_entry_name(to->m_entry), dr_entry_type_name(to->m_entry));
        return -1;
    }

    if (dr_entry_set_from_uint32(to->m_data, ui_sprite_entity_id(entity), to->m_entry, repo->m_em) != 0) {
        CPE_ERROR(
            repo->m_em, "set %s(%s) from entity-id: set with value %d fail!",
            dr_entry_name(to->m_entry), dr_entry_type_name(to->m_entry), ui_sprite_entity_id(entity));
        return -1;
    }

    return 0;
}

static int ui_sprite_data_build_set_entity_name(dr_data_entry_t to, ui_sprite_repository_t repo, ui_sprite_entity_t entity) {
    if (entity == NULL) {
        CPE_ERROR(
            repo->m_em, "set %s(%s) from entity-name: no source entity!",
            dr_entry_name(to->m_entry), dr_entry_type_name(to->m_entry));
        return -1;
    }

    if (dr_entry_set_from_string(to->m_data, ui_sprite_entity_name(entity), to->m_entry, repo->m_em) != 0) {
        CPE_ERROR(
            repo->m_em, "set %s(%s) from entity-id: set with value %s fail!",
            dr_entry_name(to->m_entry), dr_entry_type_name(to->m_entry), ui_sprite_entity_name(entity));
        return -1;
    }

    return 0;
}

int ui_sprite_data_build(
    dr_data_entry_t to, char * arg_value,
    ui_sprite_world_t world, ui_sprite_entity_t entity, dr_data_source_t data_source)
{
    ui_sprite_repository_t repo = world->m_repo;
    struct dr_data_entry from_attr_buf;
    dr_data_entry_t from_attr;

    if (arg_value[0] != '@') {
        if (dr_data_entry_set_from_string(to, arg_value, repo->m_em) != 0) {
            CPE_ERROR(
                repo->m_em, "set %s(%s) with value %s fail!", 
                dr_entry_name(to->m_entry), dr_entry_type_name(to->m_entry), arg_value);
            return -1;
        }
        return 0;
    }

    arg_value += 1;

    if (arg_value[0] == '[') {
        char * entity_name = cpe_str_trim_head(arg_value + 1);
        char * entity_name_end = strchr(entity_name, ']');
        if (entity_name_end == NULL) {
            CPE_ERROR(
                repo->m_em, "set %s(%s) with entity %s format error!",
                dr_entry_name(to->m_entry), dr_entry_type_name(to->m_entry), arg_value);
            return -1;
        }

        *cpe_str_trim_tail(entity_name_end, entity_name) = 0;
        arg_value = cpe_str_trim_head(entity_name_end + 1);

        entity = ui_sprite_entity_find_by_name(world, entity_name);
        if (entity == NULL) {
            CPE_ERROR(
                repo->m_em, "set %s(%s) with entity %s: entity not exist!",
                dr_entry_name(to->m_entry), dr_entry_type_name(to->m_entry), entity_name);
            return -1;
        }
    }

    if (strcmp(arg_value, "entity-id") == 0) {
        return ui_sprite_data_build_set_entity_id(to, repo, entity);
    }
    else if (strcmp(arg_value, "entity-name") == 0) {
        return ui_sprite_data_build_set_entity_name(to, repo, entity);
    }

    from_attr = dr_data_entry_search_in_source(&from_attr_buf, data_source, arg_value);
    if (from_attr == NULL) {
        from_attr = ui_sprite_entity_find_attr(&from_attr_buf, entity, arg_value);
    }

    if (from_attr == NULL) {
        CPE_ERROR(
            repo->m_em, "set %s(%s) from %s: source entry not exist!",
            dr_entry_name(to->m_entry), dr_entry_type_name(to->m_entry), arg_value);
        return -1;
    }

    if (dr_data_entry_set_from_entry(to, from_attr, repo->m_em) != 0) {
        CPE_ERROR(
            repo->m_em, "set %s(%s) from entry %s(%s) fail!",
            dr_entry_name(to->m_entry), dr_entry_type_name(to->m_entry), 
            dr_entry_name(from_attr->m_entry), dr_entry_type_name(from_attr->m_entry));
        return -1;
    }

    return 0;
}
