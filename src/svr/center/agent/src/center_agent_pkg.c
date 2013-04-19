#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_platform.h"
#include "cpe/dp/dp_request.h"
#include "gd/app/app_context.h"
#include "svr/center/center_agent.h"
#include "svr/center/center_agent_pkg.h"
#include "center_agent_internal_types.h"

center_agent_pkg_t
center_agent_pkg_create(center_agent_t agent, size_t pkg_capacity) {
    dp_req_t dp_req;
    center_agent_pkg_t center_agent_pkg;

    dp_req = dp_req_create(
        gd_app_dp_mgr(agent->m_app),
        center_agent_pkg_type_name,
        sizeof(struct center_agent_pkg) + pkg_capacity);
    if (dp_req == NULL) return NULL;

    bzero(dp_req_data(dp_req), dp_req_capacity(dp_req));

    center_agent_pkg = (center_agent_pkg_t)dp_req_data(dp_req);

    center_agent_pkg->m_agent = agent;
    center_agent_pkg->m_dp_req = dp_req;

    return center_agent_pkg;
}

void center_agent_pkg_free(center_agent_pkg_t req) {
    dp_req_free(req->m_dp_req);
}

center_agent_t center_agent_pkg_agent(center_agent_pkg_t req) {
    return req->m_agent;
}

size_t center_agent_pkg_capacity(center_agent_pkg_t req) {
    return dp_req_capacity(req->m_dp_req) - sizeof(struct center_agent_pkg);
}

void * center_agent_pkg_data(center_agent_pkg_t req) {
    return ((char *)(req + 1));
}

size_t center_agent_pkg_size(center_agent_pkg_t req) {
    return dp_req_size(req->m_dp_req) - sizeof(struct center_agent_pkg);
}

int center_agent_pkg_set_size(center_agent_pkg_t req, size_t size) {
    size_t total_size = size + sizeof(struct center_agent_pkg);
    if (total_size > dp_req_capacity(req->m_dp_req)) return -1;
    return dp_req_set_size(req->m_dp_req, total_size);
}

dp_req_t center_agent_pkg_to_dp_req(center_agent_pkg_t req) {
    return req->m_dp_req;
}

center_agent_pkg_t center_agent_pkg_from_dp_req(dp_req_t req) {
    if (cpe_hs_cmp(dp_req_type_hs(req), center_agent_pkg_type_name) != 0) return NULL;
    return (center_agent_pkg_t)dp_req_data(req);
}

CPE_HS_DEF_VAR(center_agent_pkg_type_name, "center_agent_pkg_type");

