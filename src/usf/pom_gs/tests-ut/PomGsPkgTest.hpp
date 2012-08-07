#ifndef USF_POM_GS_PKG_TEST_H
#define USF_POM_GS_PKG_TEST_H
#include "usf/pom_gs/pom_gs_pkg.h"
#include "PomGsAgentTest.hpp"

typedef ::Loki::NullType PomGsPkgTestBase;

class PomGsPkgTest : public testenv::fixture<PomGsPkgTestBase, PomGsAgentTest> {
public:
    PomGsPkgTest();

    void SetUp();
    void TearDown();

    void installPkg(size_t capacity = 1024);

    pom_gs_pkg_t m_pkg;
};

#endif
