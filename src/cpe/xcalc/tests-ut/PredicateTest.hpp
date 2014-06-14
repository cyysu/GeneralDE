#ifndef CPE_XCALC_PREDICATE_TEST_H
#define CPE_XCALC_PREDICATE_TEST_H
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/xcalc/xcalc_predicate.h"

class PredicateTest : public testenv::fixture<> {
public:
    virtual void SetUp();
    virtual void TearDown();

    xpredicate_t m_predicate;
};

#endif
