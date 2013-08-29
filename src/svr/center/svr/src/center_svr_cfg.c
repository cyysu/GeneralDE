#include <assert.h>
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_context.h"
#include "center_svr_ops.h"

static int center_svr_load_svr_groups(center_svr_t svr, cfg_t svr_types) {
    struct cfg_it svr_it;
    cfg_t svr_cfg;

    cfg_it_init(&svr_it, svr_types);
    while((svr_cfg = cfg_it_next(&svr_it))) {
        center_cli_group_t group;
        const char * svr_type_name = cfg_name(svr_cfg);
        uint16_t svr_type_id = cfg_get_uint16(svr_cfg, "id", 0);

        if (center_cli_group_lsearch_by_name(svr, svr_type_name) != NULL) {
            CPE_INFO(svr->m_em, "%s: load svr config: svr %s duplicate", center_svr_name(svr), svr_type_name);
            return -1;
        }

        if (svr_type_id == 0) {
            CPE_ERROR(svr->m_em, "%s: load svr config: svr %s id invalid", center_svr_name(svr), svr_type_name);
            return -1;
        }

        if (center_cli_group_find(svr, svr_type_id)) {
            CPE_ERROR(svr->m_em, "%s: load svr config: svr %s id %d duplicate!", center_svr_name(svr), svr_type_name, svr_type_id);
            return -1;
        }

        group = center_cli_group_create(svr, svr_type_name, svr_type_id);
        if (group == NULL) {
            CPE_ERROR(svr->m_em, "%s: load svr config: svr %s(%d) create fail!", center_svr_name(svr), svr_type_name, svr_type_id);
            return -1;
        }

        if (svr->m_debug) {
            CPE_INFO(svr->m_em, "%s: load svr config: svr %s(%d) load success!", center_svr_name(svr), svr_type_name, svr_type_id);
        }
    }

    return 0;
}

static int center_svr_load_svr_relations(center_svr_t svr, cfg_t svr_types) {
    struct cfg_it svr_it;
    cfg_t svr_cfg;

    cfg_it_init(&svr_it, svr_types);
    while((svr_cfg = cfg_it_next(&svr_it))) {
        center_cli_group_t user;
        center_cli_group_t provider;
        struct cfg_it provider_it;
        cfg_t provider_cfg;
        const char * user_type_name;

        user_type_name = cfg_name(svr_cfg);
        user = center_cli_group_lsearch_by_name(svr, user_type_name);
        if (user == NULL) {
            CPE_ERROR(svr->m_em, "%s: load svr config: user %s not exist!", center_svr_name(svr), user_type_name);
            return -1;
        }

        cfg_it_init(&provider_it, cfg_find_cfg(svr_cfg, "depends"));
        while((provider_cfg = cfg_it_next(&provider_it))) {
          const char * provider_type_name = cfg_as_string(provider_cfg, NULL);
          if (provider_type_name == NULL) {
            CPE_ERROR(svr->m_em, "%s: load svr config: provider %s error!", center_svr_name(svr), provider_type_name);
            return -1;
          }

          provider = center_cli_group_lsearch_by_name(svr, provider_type_name);
          if (provider == NULL) {
            CPE_ERROR(svr->m_em, "%s: load svr config: provider %s not exist!", center_svr_name(svr), provider_type_name);
            return -1;
          }

          if (center_cli_relation_create(provider, user) == NULL) {
              CPE_ERROR(
                  svr->m_em, "%s: load svr config: rel %s(%d) ==> %s(%d) create fail!",
                  center_svr_name(svr), user->m_svr_type_name, user->m_svr_type, provider->m_svr_type_name, provider->m_svr_type);
              return -1;
          }

          if (svr->m_debug) {
              CPE_INFO(
                  svr->m_em, "%s: load svr config: rel %s(%d) ==> %s(%d) load success!",
                  center_svr_name(svr), user->m_svr_type_name, user->m_svr_type, provider->m_svr_type_name, provider->m_svr_type);
          }
        }
    }

    return 0;
}

int center_svr_load_svr_config(center_svr_t svr) {
    cfg_t svr_types;

    svr_types = cfg_find_cfg(gd_app_cfg(svr->m_app), "svr_types");
    if (svr_types == NULL) {
        CPE_ERROR(svr->m_em, "%s: load svr config: no config data", center_svr_name(svr));
        return -1;
    }

    if (center_svr_load_svr_groups(svr, svr_types) != 0
        || center_svr_load_svr_relations(svr, svr_types) != 0)
    {
        return -1;
    }

    return 0;
}
