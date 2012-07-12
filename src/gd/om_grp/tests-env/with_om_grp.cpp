#include "cpe/utils/buffer.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/utils/tests-env/with_em.hpp"
#include "cpe/cfg/tests-env/with_cfg.hpp"
#include "cpe/dr/tests-env/with_dr.hpp"
#include "gd/om_grp/om_grp_meta_build.h"
#include "gd/om_grp/tests-env/with_om_grp.hpp"

namespace gd { namespace om_grp { namespace testenv {

with_om_grp::with_om_grp() {
}

const char *
with_om_grp::t_om_grp_meta_dump(om_grp_meta_t meta) {
    mem_buffer buffer;
    mem_buffer_init(&buffer, 0);

    write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(&buffer);
    om_grp_meta_dump((write_stream_t)&stream, meta, 0);
    stream_putc((write_stream_t)&stream, 0);

    const char * r = t_tmp_strdup((char *)mem_buffer_make_continuous(&buffer, 0));

    mem_buffer_clear(&buffer);

    return r;
}

om_grp_meta_t
with_om_grp::t_om_grp_meta_create(
    const char * om_meta,
    const char * str_metalib, 
    uint16_t page_size, size_t buffer_size)
{
    LPDRMETALIB metalib = 
        envOf<cpe::dr::testenv::with_dr>().t_create_metalib(
            str_metalib);
    EXPECT_TRUE(metalib) << "create metalib fail!";
    if (metalib == NULL) return NULL;

    return t_om_grp_meta_create(om_meta, metalib, page_size, buffer_size);
}

om_grp_meta_t
with_om_grp::t_om_grp_meta_create(
    const char * str_om_meta,
    LPDRMETALIB metalib, 
    uint16_t page_size, size_t buffer_size)
{
    error_monitor_t em = NULL;
    if (tryEnvOf<utils::testenv::with_em>()) {
        em = envOf<utils::testenv::with_em>().t_em();
    }

    cfg_t om_meta = envOf<cpe::cfg::testenv::with_cfg>().t_cfg_parse(str_om_meta);
    EXPECT_TRUE(om_meta) << "parse om_meta fail!";
    if (om_meta == NULL) return NULL;

    om_meta = cfg_child_only(om_meta);
    EXPECT_TRUE(om_meta) << "om_meta format error, get child only fail!";
    if (om_meta == NULL) return NULL;

    om_grp_meta_t meta =
        om_grp_meta_build_from_cfg(t_tmp_allocrator(), page_size, buffer_size, om_meta, metalib, em);
    EXPECT_TRUE(meta) << "create om_grp_meta fail!"; 
    if (meta == NULL) return NULL;

    return meta;
}

om_grp_obj_mgr_t
with_om_grp::t_om_grp_obj_mgr_create(
    const char * om_meta,
    const char * str_metalib, 
    size_t capacity, uint16_t page_size, size_t buffer_size)
{
    LPDRMETALIB metalib = 
        envOf<cpe::dr::testenv::with_dr>().t_create_metalib(
            str_metalib);
    EXPECT_TRUE(metalib) << "create metalib fail!";
    if (metalib == NULL) return NULL;

    return t_om_grp_obj_mgr_create(om_meta, metalib, capacity, page_size, buffer_size);
}

om_grp_obj_mgr_t
with_om_grp::t_om_grp_obj_mgr_create(
    const char * om_meta,
    LPDRMETALIB metalib,
    size_t capacity, uint16_t page_size, size_t buffer_size)
{
    error_monitor_t em = NULL;
    if (tryEnvOf<utils::testenv::with_em>()) {
        em = envOf<utils::testenv::with_em>().t_em();
    }

    void * buf = mem_alloc(t_tmp_allocrator(), capacity);
    EXPECT_TRUE(buf) << "malloc buf fail!";
    if (buf == NULL) return NULL;

    om_grp_meta_t meta = t_om_grp_meta_create(om_meta, metalib, page_size, buffer_size);
    if (meta == NULL) return NULL;

    int rv = om_grp_obj_mgr_buf_init(
            metalib,
            meta,
            buf, capacity,
            em);

    EXPECT_TRUE(rv == 0) << "om_grp_obj_mgr_buf_init fail!";
    if (rv != 0) return NULL;

    om_grp_obj_mgr_t mgr = om_grp_obj_mgr_create(t_allocrator(), buf, capacity, em);
    EXPECT_TRUE(mgr) << "om_mgr_obj_gr create fail!";
    return mgr;
}

}}}
