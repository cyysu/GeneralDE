#include "gd/om_grp/om_grp_obj.h"
#include "OmGrpObjMgrTest.hpp" 

class OmGrpObjBinaryTest : public OmGrpObjMgrTest {
public:
    virtual void SetUp() {
        OmGrpObjMgrTest::SetUp();
        
        install(
            "TestObj:\n"
            "  - entry1: { entry-type: binary, capacity: 5 }\n"
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

TEST_F(OmGrpObjBinaryTest, empty) {
    EXPECT_TRUE(om_grp_obj_binary(m_mgr, m_obj, "entry1") == NULL);
}

TEST_F(OmGrpObjBinaryTest, capacity) {
    EXPECT_EQ(5, om_grp_obj_binary_capacity(m_mgr, m_obj, "entry1"));
}

TEST_F(OmGrpObjBinaryTest, check_or_create) {
    EXPECT_TRUE(om_grp_obj_binary_check_or_create(m_mgr, m_obj, "entry1") != NULL);
}
