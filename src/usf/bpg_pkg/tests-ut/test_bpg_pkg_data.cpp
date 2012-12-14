#include "BpgPkgTest.hpp"

class BpgPkgDataTest : public BpgPkgTest {
public:
    BpgPkgDataTest() : m_pkg(NULL) {
    }

    virtual void SetUp() {
        BpgPkgTest::SetUp();

        m_pkg = t_bpg_pkg_create();
    }

    virtual void TearDown() {
        if (m_pkg) {
            bpg_pkg_free(m_pkg);
            m_pkg = NULL;
        }

        BpgPkgTest::TearDown();
    }

    bpg_pkg_t m_pkg;
};

TEST_F(BpgPkgDataTest, cmd_no_meta) {
    bpg_pkg_set_cmd(m_pkg, 2);
    EXPECT_EQ((uint32_t)2, bpg_pkg_cmd(m_pkg));
}
