#include "svr/set/share/set_repository.h"
#include "set_svr_ops.h"

static void set_svr_on_find_svr(void * ctx, const char * svr_type_name, uint16_t svr_id) {
    set_svr_t svr = ctx;
    set_svr_svr_t svr_svr;
    set_svr_svr_type_t svr_type;

    svr_type = set_svr_svr_type_find_by_name(svr, svr_type_name);
    if (svr_type == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: on find svr %s.%d: svr type %s not exist!",
            set_svr_name(svr), svr_type_name, svr_id, svr_type_name);
        return;
    }

    svr_svr = set_svr_svr_find(svr, svr_type->m_svr_type_id, svr_id);
    if (svr_svr) {
        if (svr_svr->m_category == set_svr_svr_local) {
            if (svr->m_debug >= 2) {
                CPE_INFO(
                    svr->m_em, "%s: on find svr %s.%d: already exist, ignore!",
                    set_svr_name(svr), svr_type_name, svr_id);
            }
        }
        else {
            CPE_INFO(
                svr->m_em, "%s: on find svr %s.%d: already exist, update to local!",
                set_svr_name(svr), svr_type_name, svr_id);

            set_svr_svr_set_category(svr_svr, set_svr_svr_local);
        }

        return;
    }

    svr_svr = set_svr_svr_create(svr, svr_type, svr_id, set_svr_svr_local);
    if (svr_svr == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: on find svr %s.%d: create fail!",
            set_svr_name(svr), svr_type_name, svr_id);
        return;
    }

    svr_svr->m_chanel = set_repository_chanel_attach(svr->m_app, svr_type_name, svr_id, svr->m_em);
    if (svr_svr->m_chanel == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: on find svr %s.%d: attach chanel fail!",
            set_svr_name(svr), svr_type_name, svr_id);
        set_svr_svr_free(svr_svr);
        return;
    }

    if (svr->m_debug) {
        CPE_INFO(
            svr->m_em, "%s: on find svr %s.%d: found new svr!",
            set_svr_name(svr), svr_type_name, svr_id);
    }
}

int set_svr_svr_search(set_svr_t svr) {
    return set_repository_search(svr->m_app, set_svr_on_find_svr, svr, svr->m_em);
}



