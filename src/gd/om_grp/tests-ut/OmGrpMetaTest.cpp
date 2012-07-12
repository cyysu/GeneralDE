#include "OmGrpMetaTest.hpp"

OmGrpMetaTest::OmGrpMetaTest() : m_meta(NULL) {
}

void OmGrpMetaTest::SetUp() {
    Base::SetUp();
}

void OmGrpMetaTest::TearDown() {
    if (m_meta) {
        om_grp_meta_free(m_meta);
        m_meta = NULL;
    }

    Base::TearDown();
}

void OmGrpMetaTest::install(const char * om_meta, const char * metalib, uint16_t page_size) {
    if (m_meta) {
        om_grp_meta_free(m_meta);
        m_meta = NULL;
    }

    m_meta = t_om_grp_meta_create(om_meta, metalib, page_size);
}
