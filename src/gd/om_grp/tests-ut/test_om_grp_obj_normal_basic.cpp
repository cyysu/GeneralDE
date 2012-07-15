#include "gd/om_grp/om_grp_obj.h"
#include "OmGrpObjMgrTest.hpp" 

class OmGrpObjNormalTest : public OmGrpObjMgrTest {
public:
    virtual void SetUp() {
        OmGrpObjMgrTest::SetUp();
        
        install(
            "TestObj:\n"
            "  - entry1: { entry-type: normal, data-type: AttrGroup1 }\n"
            ,
            "<metalib tagsetversion='1' name='net'  version='1'>"
            "    <struct name='AttrGroup1' version='1'>"
            "	     <entry name='a1' type='uint32' id='1'/>"
            "    </struct>"
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

TEST_F(OmGrpObjNormalTest, empty) {
    EXPECT_TRUE(om_grp_obj_normal(m_mgr, m_obj, "entry1") == NULL);
}

TEST_F(OmGrpObjNormalTest, check_or_create) {
    t_em_set_print();
    EXPECT_TRUE(om_grp_obj_normal_check_or_create(m_mgr, m_obj, "entry1") != NULL);
}
