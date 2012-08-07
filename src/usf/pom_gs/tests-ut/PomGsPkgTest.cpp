#include "PomGsPkgTest.hpp"

PomGsPkgTest::PomGsPkgTest()
  : m_pkg(NULL)
{
}

void PomGsPkgTest::SetUp() {
    Base::SetUp();
}

void PomGsPkgTest::TearDown() {
    if (m_pkg) pom_gs_pkg_free(m_pkg);

    Base::TearDown();
}

void PomGsPkgTest::installPkg(size_t capacity) {
    if (m_pkg) pom_gs_pkg_free(m_pkg);

    m_pkg = pom_gs_pkg_create(m_agent, capacity);
}
