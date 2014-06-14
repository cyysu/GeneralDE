#include "cpe/utils/buffer.h"
#include "PredicateTest.hpp"

void PredicateTest::SetUp() {
    Base::SetUp();
    m_predicate = NULL;
}

void PredicateTest::TearDown() {
    if (m_predicate) {
        xpredicate_free(t_allocrator(), m_predicate);
        m_predicate = NULL;
    }

    Base::TearDown();
}

const char * PredicateTest::sort(void) {
    const char * r = NULL;
    return sort(r) == 0 ? r : NULL;
}

int PredicateTest::sort(const char * & str) {
    struct mem_buffer buffer;
    struct tsorter_str_it it;

    int r = tsorter_str_sort(&it, m_sorter);

    mem_buffer_init(&buffer, NULL);

    while(const char * d = tsorter_str_next(&it)) {
        mem_buffer_strcat(&buffer, d);
        mem_buffer_strcat(&buffer, ":");
    }

    mem_buffer_strcat(&buffer, "");

    str = t_tmp_strdup((const char *)mem_buffer_make_continuous(&buffer, 0));

    mem_buffer_clear(&buffer);

    return r;
}

void PredicateTest::addDepend(const char * dep_from, const char * dep_to) {
    EXPECT_TRUE(tsorter_str_add_dep(m_sorter, dep_from, dep_to) == 0);
}
