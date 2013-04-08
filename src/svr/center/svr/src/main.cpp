#include "argtable2.h"
#include "cpe/pal/pal_unistd.h"
#include "cpe/pal/pal_dlfcn.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_shm.h"
#include "cpe/utils/stream_file.h"
#include "cpe/dr/dr_metalib_init.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/pom_grp/pom_grp_meta.h"
#include "cpe/pom_grp/pom_grp_shm.h"
#include "cpepp/cfg/Node.hpp"
#include "gdpp/app/Application.hpp"

static int svr_main(int argc, char * argv[], int shmkey) {
    gd_app_context_t ctx;
    int rv;

    ctx = gd_app_context_create_main(NULL, 0, argc, argv);
    if (ctx == NULL) return -1;

    gd_app_set_debug(ctx, 1);

    Gd::App::Application::_cast(ctx).cfg()["shmkey"] = shmkey;

    gd_set_default_library(dlopen(NULL, RTLD_NOW));

	rv = gd_app_run(ctx);

	gd_app_context_free(ctx);

    return rv;
}

int main(int argc, char * argv[]) {
    return svr_main(argc, argv, 0);
} 
