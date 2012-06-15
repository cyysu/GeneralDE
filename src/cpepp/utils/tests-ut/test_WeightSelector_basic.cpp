#include "WeightSelectorTest.hpp"

TEST_F(WeightSelectorTest, empty) {
    EXPECT_EQ(-1, select());
}

TEST_F(WeightSelectorTest, select_basic) {
    add(3);
    add(3);

    t_random_expect_gen_with_arg(6, 0);

    EXPECT_EQ(0, select());
}

TEST_F(WeightSelectorTest, select_one_basic) {
    add(3);

    t_random_expect_gen_with_arg(3, 3);

    EXPECT_EQ(-1, select());
}

TEST_F(WeightSelectorTest, select_random_overflow) {
    add(3);
    add(3);

    t_random_expect_gen_with_arg(6, 0);
    EXPECT_EQ(0, select());

    t_random_expect_gen_with_arg(6, 1);
    EXPECT_EQ(0, select());

    t_random_expect_gen_with_arg(6, 2);
    EXPECT_EQ(0, select());

    t_random_expect_gen_with_arg(6, 3);
    EXPECT_EQ(1, select());

    t_random_expect_gen_with_arg(6, 4);
    EXPECT_EQ(1, select());

    t_random_expect_gen_with_arg(6, 5);
    EXPECT_EQ(1, select());
}

TEST_F(WeightSelectorTest, select_zero) {
    add(0);

    t_random_expect_gen_with_arg(1, 0);

    EXPECT_EQ(0, select());
}

TEST_F(WeightSelectorTest, select_zero_multi) {
    add(0);
    add(0);

    t_random_expect_gen_with_arg(2, 1);

    EXPECT_EQ(1, select());
}

TEST_F(WeightSelectorTest, select_zero_overflow) {
    add(0);
    add(0);

    t_random_expect_gen_with_arg(2, 2);

    EXPECT_EQ(-1, select());
}
