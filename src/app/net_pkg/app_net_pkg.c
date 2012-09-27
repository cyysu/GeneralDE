#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/dr/dr_metalib_init.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_json.h"
#include "cpe/dp/dp_request.h"
#include "gd/app/app_context.h"
#include "gd/dr_store/dr_ref.h"
#include "gd/dr_store/dr_store_manage.h"
#include "gd/dr_cvt/dr_cvt.h"
#include "app/net_pkg/app_net_pkg.h"
#include "app/net_pkg/app_net_pkg_manage.h"
#include "app_net_pkg_internal_types.h"

app_net_pkg_t
app_net_pkg_create(
    app_net_pkg_manage_t mgr,
    size_t pkg_capacity)
{
    dp_req_t dp_req;
    app_net_pkg_t app_net_pkg;

    dp_req = dp_req_create(
        gd_app_dp_mgr(mgr->m_app),
        app_net_pkg_type_name,
        sizeof(struct app_net_pkg));
    if (dp_req == NULL) return NULL;

    app_net_pkg = (app_net_pkg_t)dp_req_data(dp_req);

    app_net_pkg->m_mgr = mgr;
    app_net_pkg->m_dp_req = dp_req;

    app_net_pkg_init(app_net_pkg);

    return app_net_pkg;
}

void app_net_pkg_free(app_net_pkg_t req) {
    dp_req_free(req->m_dp_req);
}

app_net_pkg_manage_t app_net_pkg_mgr(app_net_pkg_t req) {
    return req->m_mgr;
}

size_t app_net_pkg_data_capacity(app_net_pkg_t req) {
    return dp_req_capacity(req->m_dp_req) - sizeof(struct app_net_pkg);
}

void * app_net_pkg_data(app_net_pkg_t req) {
    return ((char *)(req + 1));
}

size_t app_net_pkg_data_size(app_net_pkg_t req) {
    return dp_req_size(req->m_dp_req) - sizeof(struct app_net_pkg);
}

int app_net_pkg_data_set_size(app_net_pkg_t req, size_t size) {
    size_t total_size = size + sizeof(struct app_net_pkg);
    if (total_size > dp_req_capacity(req->m_dp_req)) return -1;
    return dp_req_set_size(req->m_dp_req, total_size);
}

void app_net_pkg_init(app_net_pkg_t app_net_pkg) {
    /* TSPKGDATA_HEAD * head; */

    /* app_net_pkg_data_set_size(app_net_pkg, sizeof(TSPKGDATA_HEAD)); */

    /* app_net_pkg->m_connection_id = BPG_INVALID_CONNECTION_ID; */

    /* head = (TSPKGDATA_HEAD *)app_net_pkg_data(app_net_pkg); */
    /* bzero(head, sizeof(TSPKGDATA_HEAD)); */
    /* head->magic = TSPKGDATA_HEAD_MAGIC; */
    /* head->version = 1; */
}

CPE_HS_DEF_VAR(app_net_pkg_type_name, "app_net_pkg_type");

