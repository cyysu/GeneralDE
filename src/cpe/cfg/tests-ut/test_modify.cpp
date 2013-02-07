#include "ModifyTest.hpp"

TEST_F(ModifyTest, set_value_basic) {
    t_em_set_print();

    EXPECT_STREQ(
        "{ a=13 }"
        ,
        modify("a: 12", "set: { a: 13 }"));
}

