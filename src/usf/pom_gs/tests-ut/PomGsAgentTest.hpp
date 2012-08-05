#ifndef USF_POM_GS_PKG_TEST_H
#define USF_POM_GS_PKG_TEST_H
#include "gmock/gmock.h"
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/tests-env/with_em.hpp"
#include "cpe/cfg/tests-env/with_cfg.hpp"
#include "cpe/pom_grp/tests-env/with_pom_grp.hpp"
#include "gd/app/tests-env/with_app.hpp"
#include "usf/pom_gs/tests-env/with_pom_gs.hpp"

typedef LOKI_TYPELIST_5(
    utils::testenv::with_em
    , gd::app::testenv::with_app
    , cpe::cfg::testenv::with_cfg
    , cpe::pom_grp::testenv::with_pom_grp
    , usf::pom_gs::testenv::with_pom_gs
    ) PomGsAgentTestBase;

class PomGsAgentTest : public testenv::fixture<PomGsAgentTestBase> {
public:
    class BackendMock {
    public:
        MOCK_METHOD1(execute, logic_op_exec_result_t(logic_stack_node_t node));
    };

    PomGsAgentTest();

    void SetUp();
    void TearDown();

    void installAgent(
        const char * om_meta,
        const char * metalib, 
        const char * main_entry,
        const char * key);

    pom_grp_meta_t m_pom_grp_meta;
    pom_gs_agent_t m_agent;
};

#endif
