#include "usf/pom_gs/pom_gs_pkg_cfg.h"
#include "PomGsPkgTest.hpp"

class PomGsPkgBasicTest : public PomGsPkgTest {
public:
    void SetUp() {
        PomGsPkgTest::SetUp();

        installAgent(
            "TestObj:\n"
            "  - entry1: { entry-type: normal, data-type: AttrGroup1 }\n"
            "  - entry2: { entry-type: list, data-type: AttrGroup2, group-count: 3, capacity: 3 }\n"
            "  - entry3: { entry-type: ba, bit-capacity: 15 }\n"
            "  - entry4: { entry-type: binary, capacity: 5 }\n"
            ,
            "<metalib tagsetversion='1' name='net'  version='1'>"
            "    <struct name='AttrGroup1' version='1'>"
            "	     <entry name='a1' type='uint32' id='1'/>"
            "    </struct>"
            "    <struct name='AttrGroup2' version='1'>"
            "	     <entry name='b1' type='string' size='5' id='1'/>"
            "    </struct>"
            "</metalib>"
            ,
            "entry1"
            ,
            "a1");

        installPkg();
    }
};

TEST_F(PomGsPkgBasicTest, basic) {
    EXPECT_EQ(0, pom_gs_pkg_table_buf_capacity(m_pkg, "AttrGroup1"));

    size_t capacity = 0;
    EXPECT_TRUE(pom_gs_pkg_table_buf(m_pkg, "AttrGroup1", &capacity));
    EXPECT_EQ((size_t)30, capacity);
    EXPECT_EQ(30, pom_gs_pkg_table_buf_capacity(m_pkg, "AttrGroup1"));
}

TEST_F(PomGsPkgBasicTest, not_exist) {
    EXPECT_EQ(0, pom_gs_pkg_table_buf_capacity(m_pkg, "not-exist"));

    size_t capacity = 0;
    EXPECT_TRUE(NULL == pom_gs_pkg_table_buf(m_pkg, "not-exist", &capacity));
    EXPECT_EQ((size_t)0, capacity);
    EXPECT_EQ(0, pom_gs_pkg_table_buf_capacity(m_pkg, "not-exist"));
}

TEST_F(PomGsPkgBasicTest, cfg_basic) {
    t_em_set_print();

    t_pom_gs_pkg_load(
        m_pkg,
        "AttrGroup1:\n"
        "    a1: 1"
        );

    EXPECT_CFG_EQ_PART(
        "AttrGroup1:\n"
        "    a1: 1"
        ,
        t_pom_gs_pkg_dump_to_cfg(m_pkg));
}
