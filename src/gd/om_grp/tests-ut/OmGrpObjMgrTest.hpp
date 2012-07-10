#ifndef CPE_OM_GRP_TEST_MGRTEST_H
#define CPE_OM_GRP_TEST_MGRTEST_H
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/tests-env/with_em.hpp"
#include "cpe/dr/tests-env/with_dr.hpp"
#include "gd/om_grp/tests-env/with_om_grp.hpp"

typedef LOKI_TYPELIST_3(
    utils::testenv::with_em,
    cpe::dr::testenv::with_dr,
    gd::om_grp::testenv::with_om_grp) OmGrpObjMgrTestBase;

class OmGrpObjMgrTest : public testenv::fixture<OmGrpObjMgrTestBase> {
public:
    OmGrpObjMgrTest();

    virtual void SetUp();
    virtual void TearDown();

    void install(
        const char * om_meta,
        const char * metalib, 
        size_t capacity = 2048, uint16_t page_size = 256, size_t buffer_size = 512);

    om_grp_obj_mgr_t m_mgr;
};

#endif
