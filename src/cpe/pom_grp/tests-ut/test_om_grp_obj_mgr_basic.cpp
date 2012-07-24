#include "OmGrpObjMgrTest.hpp" 

class OmGrpObjMgrBasicTest : public OmGrpObjMgrTest {
public:
    virtual void SetUp() {
        OmGrpObjMgrTest::SetUp();

        t_em_set_print();

        install(
            "TestObj:\n"
            "  - entry1: { entry-type: normal, data-type: AttrGroup1 }\n"
            "  - entry2: { entry-type: list, data-type: AttrGroup2, group-count: 3, capacity: 3 }\n"
            "  - entry3: { entry-type: binary, capacity: 5 }\n"
            ,
            "<metalib tagsetversion='1' name='net'  version='1'>"
            "    <struct name='AttrGroup1' version='1'>"
            "	     <entry name='a1' type='uint32' id='1'/>"
            "    </struct>"
            "    <struct name='AttrGroup2' version='1'>"
            "	     <entry name='b1' type='string' size='5' id='1'/>"
            "    </struct>"
            "</metalib>"
            ) ;
            
    }
};

TEST_F(OmGrpObjMgrBasicTest, obj_basic) {
    pom_grp_obj_t obj = pom_grp_obj_alloc(m_mgr);
    ASSERT_TRUE(obj);
}

TEST_F(OmGrpObjMgrBasicTest, obj_it_basic) {
    pom_grp_obj_t obj = pom_grp_obj_alloc(m_mgr);
    ASSERT_TRUE(obj);

    pom_grp_obj_it it;
    pom_grp_objs(m_mgr, &it);

    ASSERT_TRUE(pom_grp_obj_it_next(&it) == obj);
    ASSERT_TRUE(pom_grp_obj_it_next(&it) == NULL);
}
