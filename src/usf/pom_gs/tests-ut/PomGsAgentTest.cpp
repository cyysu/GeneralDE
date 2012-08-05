#include <stdexcept>
#include "usf/logic/logic_executor_type.h"
#include "PomGsAgentTest.hpp"

PomGsAgentTest::PomGsAgentTest()
  : m_pom_grp_meta(NULL), m_agent(NULL)
{
}

void PomGsAgentTest::SetUp() {
    Base::SetUp();
}

void PomGsAgentTest::TearDown() {
    if (m_agent) pom_gs_agent_free(m_agent);
    if (m_pom_grp_meta) pom_grp_meta_free(m_pom_grp_meta);

    Base::TearDown();
}

void PomGsAgentTest::installAgent(
    const char * om_meta,
    const char * metalib, 
    const char * main_entry,
    const char * key)
{
    if (m_agent) { pom_gs_agent_free(m_agent); m_agent = NULL; }
    if (m_pom_grp_meta) { pom_grp_meta_free(m_pom_grp_meta); m_pom_grp_meta = NULL; }

    m_pom_grp_meta = t_pom_grp_meta_create(om_meta, metalib);
    ASSERT_TRUE(m_pom_grp_meta);
    if (m_pom_grp_meta == NULL) return;

    m_agent = pom_gs_agent_create(
        t_app(), 
        "TestPomGrpAgent",
        m_pom_grp_meta,
        main_entry,
        key,
        t_allocrator(),
        t_em());
    ASSERT_TRUE(m_agent);
}



