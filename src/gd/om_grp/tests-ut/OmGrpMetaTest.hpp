#ifndef CPE_OM_GRP_TEST_META_H
#define CPE_OM_GRP_TEST_META_H
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/tests-env/with_em.hpp"
#include "cpe/cfg/tests-env/with_cfg.hpp"
#include "cpe/dr/tests-env/with_dr.hpp"
#include "gd/om_grp/tests-env/with_om_grp.hpp"

typedef LOKI_TYPELIST_4(
    utils::testenv::with_em,
    cpe::cfg::testenv::with_cfg,
    cpe::dr::testenv::with_dr,
    gd::om_grp::testenv::with_om_grp) OmGrpMetaTestBase;

class OmGrpMetaTest : public testenv::fixture<OmGrpMetaTestBase> {
public:
    OmGrpMetaTest();

    virtual void SetUp();
    virtual void TearDown();

    void install(
        const char * om_meta,
        const char * metalib, 
        uint16_t page_size = 256, size_t buffer_size = 512);

    om_grp_meta_t m_meta;
};

#endif
