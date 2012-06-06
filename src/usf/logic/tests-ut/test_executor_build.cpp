#include "LogicTest.hpp"

class ExecutorBuildTest : public LogicTest {
public:
};

TEST_F(ExecutorBuildTest, basic_basic) {
    installOp("Op1");

    logic_executor_t executor =
          t_logic_executor_build("Op1");
    ASSERT_TRUE(executor);

    EXPECT_STREQ(
        "Op1",
        t_logic_executor_dump(executor));

    logic_executor_free(executor);
}

TEST_F(ExecutorBuildTest, basic_with_args) {
    installOp("Op1");

    logic_executor_t executor =
          t_logic_executor_build("Op1: { a1: 1, a2: 2}");
    ASSERT_TRUE(executor);

    EXPECT_STREQ(
        "Op1: { a1=1, a2=2 }",
        t_logic_executor_dump(executor));

    logic_executor_free(executor);
}

TEST_F(ExecutorBuildTest, sequence_basic) {
    installOp("Op1");
    installOp("Op2");

    logic_executor_t executor =
        t_logic_executor_build(
            "- Op1\n"
            "- Op2: { a1: 1, a2: 2 }\n"
            );
    ASSERT_TRUE(executor);

    EXPECT_STREQ(
        "sequence:\n"
        "    Op1\n"
        "    Op2: { a1=1, a2=2 }"
        ,
        t_logic_executor_dump(executor));

    logic_executor_free(executor);
}

TEST_F(ExecutorBuildTest, sequence_sequence) {
    installOp("Op1");
    installOp("Op2");

    logic_executor_t executor =
        t_logic_executor_build(
            "- Op1\n"
            "-\n"
            "    - Op2: { a1: 1, a2: 2 }\n"
            );
    ASSERT_TRUE(executor);

    EXPECT_STREQ(
        "sequence:\n"
        "    Op1\n"
        "    sequence:\n"
        "        Op2: { a1=1, a2=2 }"
        ,
        t_logic_executor_dump(executor));

    logic_executor_free(executor);
}

TEST_F(ExecutorBuildTest, protected_basic) {
    installOp("Op1");

    logic_executor_t executor =
        t_logic_executor_build(
            "protect: Op1");
    ASSERT_TRUE(executor);

    EXPECT_STREQ(
        "protect:\n"
        "    Op1",
        t_logic_executor_dump(executor));

    logic_executor_free(executor);
}

TEST_F(ExecutorBuildTest, protected_sequence) {
    installOp("Op1");

    logic_executor_t executor =
        t_logic_executor_build(
            "protect:\n"
            "    - Op1");
    ASSERT_TRUE(executor);

    EXPECT_STREQ(
        "protect:\n"
        "    sequence:\n"
        "        Op1"
        ,
        t_logic_executor_dump(executor));

    logic_executor_free(executor);
}
