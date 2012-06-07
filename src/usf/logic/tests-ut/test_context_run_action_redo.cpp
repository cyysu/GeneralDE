#include "usf/logic/logic_require.h"
#include "RunTest.hpp"

class ContextRunActionRedoTest : public RunTest {
};

TEST_F(ContextRunActionRedoTest, basic) {
    LogicOpMock & op1 = installOp("Op1");
    expect_create_require(op1, logic_op_exec_result_redo);

    execute("Op1");

    EXPECT_EQ(logic_context_state_waiting, state());
    EXPECT_EQ((int32_t)0, rv());
}

TEST_F(ContextRunActionRedoTest, redo) {
    LogicOpMock & op1 = installOp("Op1");
    expect_create_require(op1, logic_op_exec_result_redo);

    execute("Op1");
    EXPECT_EQ(logic_context_state_waiting, state());
}

TEST_F(ContextRunActionRedoTest, basic_not_waiting) {
    LogicOpMock & op1 = installOp("Op1");
    expect_return(op1, logic_op_exec_result_redo);

    expect_commit();
    execute("Op1");

    EXPECT_EQ(logic_context_state_error, state());
    EXPECT_EQ((int32_t)-1, rv());
}

