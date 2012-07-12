#include "gd/om_grp/om_grp_obj.h"
#include "OmGrpObjMgrTest.hpp" 

class OmGrpObjListTest : public OmGrpObjMgrTest {
public:
    struct AttrGroup1 {
        uint32_t a1;
    };

    virtual void SetUp() {
        OmGrpObjMgrTest::SetUp();
        
        install(
            "TestObj:\n"
            "  - entry1: { entry-type: list, data-type: AttrGroup1, group-count: 3, capacity: 5 }\n"
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

TEST_F(OmGrpObjListTest, count_empty) {
    EXPECT_EQ(
        0,
        om_grp_obj_list_count(m_mgr, m_obj, "entry1"));
}

TEST_F(OmGrpObjListTest, at_overflow_capacity) {
    EXPECT_TRUE(om_grp_obj_list_at(m_mgr, m_obj, "entry1", 6) == NULL);
}

TEST_F(OmGrpObjListTest, at_overflow_count) {
    EXPECT_TRUE(om_grp_obj_list_at(m_mgr, m_obj, "entry1", 0) == NULL);
}

TEST_F(OmGrpObjListTest, at_overflow_count_not_empty) {
    AttrGroup1 v1;
    v1.a1 = 1;
    EXPECT_EQ(
        0,
        om_grp_obj_list_append(m_mgr, m_obj, "entry1", &v1));

    EXPECT_TRUE(om_grp_obj_list_at(m_mgr, m_obj, "entry1", 1) == NULL);
}

TEST_F(OmGrpObjListTest, append_basic) {
    AttrGroup1 v1;
    v1.a1 = 1;
    EXPECT_EQ(
        0,
        om_grp_obj_list_append(m_mgr, m_obj, "entry1", &v1));

    EXPECT_EQ(
        1,
        om_grp_obj_list_count(m_mgr, m_obj, "entry1"));

    AttrGroup1 * r = (AttrGroup1 *)om_grp_obj_list_at(m_mgr, m_obj, "entry1", 0);
    ASSERT_TRUE(r);
    EXPECT_EQ(1, r->a1);
}
