#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/net/net_connector.h"
#include "cpe/dp/dp_manage.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "svr/set/stub/set_svr_stub.h"
#include "svr/set/stub/set_svr_svr_info.h"
#include "dblog_svr_ops.h"

extern char g_metalib_svr_dblog_pro[];
static int dblog_svr_start_listen(dblog_svr_t dblog_svr, uint16_t port);

EXPORT_DIRECTIVE
int dblog_svr_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    set_svr_stub_t stub;
    dblog_svr_t dblog_svr;
    const char * str_port;
    uint16_t port;

    str_port = gd_app_arg_find(app, "--port");
    if (str_port == NULL) {
        CPE_ERROR(gd_app_em(app), "%s: create: port not configured", gd_app_module_name(module));
        return -1;
    }
    port = atoi(str_port);

    stub = set_svr_stub_find_nc(app, cfg_get_string(cfg, "set-stub", NULL));
    if (stub == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: set-stub %s not exist!",
            gd_app_module_name(module), cfg_get_string(cfg, "set-stub", "default"));
        return -1;
    }

    dblog_svr = dblog_svr_create(app, gd_app_module_name(module), stub, gd_app_alloc(app), gd_app_em(app));
    if (dblog_svr == NULL) return -1;

    dblog_svr->m_debug = cfg_get_int8(cfg, "debug", dblog_svr->m_debug);

    dblog_svr_start_listen(dblog_svr, port);

    if (dblog_svr->m_debug) {
        CPE_INFO(gd_app_em(app), "%s: create: done, listen at %d.", gd_app_module_name(module), port);
    }

    return 0;
}

EXPORT_DIRECTIVE
void dblog_svr_app_fini(gd_app_context_t app, gd_app_module_t module) {
    dblog_svr_t dblog_svr;

    dblog_svr = dblog_svr_find_nc(app, gd_app_module_name(module));
    if (dblog_svr) {
        dblog_svr_free(dblog_svr);
    }
}

static int dblog_svr_start_listen(dblog_svr_t dblog_svr, uint16_t port) {
    int fd;
    struct sockaddr_in addr;
    //int addr_len = sizeof(addr);

    fd = socket(PF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        CPE_ERROR(
            dblog_svr->m_em, "%s: start listen: socket error, errno=%d (%s)",
            dblog_svr_name(dblog_svr), errno, strerror(errno));
        return -1;
    }

    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(fd, (struct sockaddr*) &addr, sizeof(addr)) != 0) {
        CPE_ERROR(
            dblog_svr->m_em, "%s: start listen: bind error, port=%d, errno=%d (%s)",
            dblog_svr_name(dblog_svr), port, errno, strerror(errno));
        close(fd);
        return -1;
    }

    dblog_svr->m_fd = fd;

    ev_io_init(&dblog_svr->m_watcher, dblog_svr_net_cb, dblog_svr->m_fd, EV_READ);
    dblog_svr->m_watcher.data = dblog_svr;
    ev_io_start(dblog_svr->m_ev_loop, &dblog_svr->m_watcher);

    return 0;
}
