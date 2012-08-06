#include "PomGsAgentTest.hpp"

class PomGsAgentBasicTest : public PomGsAgentTest {
public:
    void SetUp() {
        PomGsAgentTest::SetUp();

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
    }
};

TEST_F(PomGsAgentBasicTest, basic) {
}
