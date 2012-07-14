#include "gd/om_grp/om_grp_obj.h"
#include "OmGrpObjMgrTest.hpp" 

class OmGrpObjBitarryyTest : public OmGrpObjMgrTest {
public:
    virtual void SetUp() {
        OmGrpObjMgrTest::SetUp();
        
        install(
            "TestObj:\n"
            "  - entry1: { entry-type: ba, bit-capacity: 30, byte-per-page=2 }\n"
            ,
            "<metalib tagsetversion='1' name='net'  version='1'>"
            "</metalib>"
            ) ;

        m_obj = om_mgr_obj_alloc(m_mgr);
        ASSERT_TRUE(m_obj);
    }

    virtual void TearDown() {
        om_grp_obj_free(m_mgr, m_obj);
        OmGrpObjMgrTest::TearDown();
    }

    om_grp_obj_t m_obj;
};

TEST_F(OmGrpObjBitarryyTest, capacity) {
    EXPECT_EQ(30, om_grp_obj_ba_bit_capacity(m_mgr, m_obj, "entry1"));
}

