#include "OmGrpObjMgrTest.hpp"

OmGrpObjMgrTest::OmGrpObjMgrTest() : m_mgr(NULL) {
}

void OmGrpObjMgrTest::SetUp() {
    Base::SetUp();
}

void OmGrpObjMgrTest::TearDown() {
    if (m_mgr) {
        om_grp_obj_mgr_free(m_mgr);
        m_mgr = NULL;
    }

    Base::TearDown();
}

void OmGrpObjMgrTest::install(const char * om_meta, const char * metalib, size_t capacity, uint16_t page_size, size_t buffer_size) {
    if (m_mgr) {
        om_grp_obj_mgr_free(m_mgr);
        m_mgr = NULL;
    }

    m_mgr = t_om_grp_obj_mgr_create(om_meta, metalib, capacity, page_size, buffer_size);
}
