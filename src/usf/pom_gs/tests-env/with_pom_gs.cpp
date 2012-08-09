#include <stdexcept>
#include "cpe/utils/buffer.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/utils/tests-env/with_em.hpp"
#include "cpe/cfg/tests-env/with_cfg.hpp"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/tests-env/with_app.hpp"
#include "usf/pom_gs/tests-env/with_pom_gs.hpp"

namespace usf { namespace pom_gs { namespace testenv {

with_pom_gs::with_pom_gs() {
}

void with_pom_gs::SetUp() {
}

void with_pom_gs::TearDown() {
}

const char * with_pom_gs::t_pom_gs_pkg_dump(pom_gs_pkg_t pkg) {
    error_monitor_t em = NULL;
    if (tryEnvOf<utils::testenv::with_em>()) {
        em = envOf<utils::testenv::with_em>().t_em();
    }

    mem_buffer buffer;
    mem_buffer_init(&buffer, 0);

    write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(&buffer);
    EXPECT_EQ(0, pom_gs_pkg_dump_to_stream((write_stream_t)&stream, pkg, em));
    stream_putc((write_stream_t)&stream, 0);

    const char * r = t_tmp_strdup((char *)mem_buffer_make_continuous(&buffer, 0));

    mem_buffer_clear(&buffer);

    return r;
}

void with_pom_gs::t_pom_gs_pkg_dump_to_cfg(cfg_t cfg, pom_gs_pkg_t pkg) {
    error_monitor_t em = NULL;
    if (tryEnvOf<utils::testenv::with_em>()) {
        em = envOf<utils::testenv::with_em>().t_em();
    }

    EXPECT_EQ(0, pom_gs_pkg_cfg_dump(cfg, pkg, em));
}

cfg_t with_pom_gs::t_pom_gs_pkg_dump_to_cfg(pom_gs_pkg_t pkg) {
    cfg_t cfg_data = envOf<cpe::cfg::testenv::with_cfg>().t_cfg_create();
    t_pom_gs_pkg_dump_to_cfg(cfg_data, pkg);
    return cfg_data;
}

void with_pom_gs::t_pom_gs_pkg_load(pom_gs_pkg_t pkg, cfg_t cfg) {
    error_monitor_t em = NULL;
    if (tryEnvOf<utils::testenv::with_em>()) {
        em = envOf<utils::testenv::with_em>().t_em();
    }

    EXPECT_EQ(0, pom_gs_pkg_cfg_load(pkg, cfg, em));
}

void with_pom_gs::t_pom_gs_pkg_load(pom_gs_pkg_t pkg, const char * cfg) {
    cfg_t cfg_data = envOf<cpe::cfg::testenv::with_cfg>().t_cfg_parse(cfg);
    ASSERT_TRUE(cfg_data) << "parse cfg fail!";

    t_pom_gs_pkg_load(pkg, cfg_data);
}

}}}
