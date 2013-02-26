#include "cpe/pom_grp/pom_grp_meta.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_metalib_xml.h"
#include "OmGrpStoreTest.hpp"

OmGrpStoreTest::OmGrpStoreTest() : m_meta(NULL), m_store(NULL) {
}

void OmGrpStoreTest::SetUp() {
    Base::SetUp();
}

void OmGrpStoreTest::TearDown() {
    if (m_store) {
        pom_grp_store_free(m_store);
        m_store = NULL;
    }

    if (m_meta) {
        pom_grp_meta_free(m_meta);
        m_meta = NULL;
    }

    Base::TearDown();
}

void OmGrpStoreTest::install(const char * om_meta, const char * metalib, const char * store_meta, const char * main_entry, const char * key) {
    if (m_store) {
        pom_grp_store_free(m_store);
        m_store = NULL;
    }

    if (m_meta) {
        pom_grp_meta_free(m_meta);
        m_meta = NULL;
    }

    t_em_set_print();
    m_meta = t_pom_grp_meta_create(om_meta, metalib, 1024);
    ASSERT_TRUE(m_meta);

    struct mem_buffer metalib_buffer;
    mem_buffer_init(&metalib_buffer, t_tmp_allocrator());
    EXPECT_EQ(0, pom_grp_meta_build_store_meta(&metalib_buffer, pom_grp_store_meta(m_store), t_em()));

    m_store = pom_grp_store_create(
        t_allocrator(),
        m_meta,
        dr_lib_find_meta_by_name(
            (LPDRMETALIB)(mem_buffer_make_continuous(&metalib_buffer, 0)),
            pom_grp_meta_name(m_meta)),
        t_em());
    ASSERT_TRUE(m_store);
}

const char *
OmGrpStoreTest::store_meta(void) {
    EXPECT_TRUE(m_store);
    if (m_store == NULL) return "no-store";

    struct mem_buffer metalib_buffer;
    mem_buffer_init(&metalib_buffer, t_tmp_allocrator());
    EXPECT_EQ(0, pom_grp_meta_build_store_meta(&metalib_buffer, pom_grp_store_meta(m_store), t_em()));

    struct mem_buffer buffer;
    mem_buffer_init(&buffer, t_tmp_allocrator());

    char * r = dr_save_lib_to_xml_buf(&buffer, (LPDRMETALIB)(mem_buffer_make_continuous(&metalib_buffer, 0)), t_em());
    EXPECT_TRUE(r);
    if (r == NULL) return "save-metalib-fail";

    r = t_tmp_strdup(r);

    mem_buffer_clear(&buffer);

    return r;
}
