#include "RankTreeTest.hpp"

class ContextBasicTest : public RankTreeTest {
    virtual void SetUp() {
        RankTreeTest::SetUp();
        t_rank_tree_create(100);
    }
};

TEST_F(ContextBasicTest, empty) {
    EXPECT_EQ((uint32_t)0, rt_size(m_rank_tree));
    EXPECT_EQ((uint32_t)100, rt_capacity(m_rank_tree));

    EXPECT_TRUE(rt_first(m_rank_tree) == NULL);
    EXPECT_TRUE(rt_last(m_rank_tree) == NULL);
}


TEST_F(ContextBasicTest, insert_first) {
    rt_node_t node = rt_insert(m_rank_tree, 5, 1);
    ASSERT_TRUE(node != NULL);

    EXPECT_TRUE(rt_first(m_rank_tree) == node);
    EXPECT_TRUE(rt_last(m_rank_tree) == node);

    EXPECT_EQ((uint32_t)1, rt_size(m_rank_tree));
}

TEST_F(ContextBasicTest, insert_multi) {
    rt_node_t node1 = rt_insert(m_rank_tree, 5, 1);
    ASSERT_TRUE(node1 != NULL);
    EXPECT_EQ((uint32_t)5, rt_node_value(node1));

    rt_node_t node2 = rt_insert(m_rank_tree, 7, 2);
    ASSERT_TRUE(node2 != NULL);
    EXPECT_EQ((uint32_t)7, rt_node_value(node2));

    rt_node_t node3 = rt_insert(m_rank_tree, 2, 3);
    ASSERT_TRUE(node3 != NULL);
    EXPECT_EQ((uint32_t)2, rt_node_value(node3));

    EXPECT_TRUE(rt_first(m_rank_tree) == node2);
    EXPECT_TRUE(rt_last(m_rank_tree) == node3);

    EXPECT_EQ((uint32_t)3, rt_size(m_rank_tree));
}

TEST_F(ContextBasicTest, move) {
    rt_node_t node1 = rt_insert(m_rank_tree, 5, 1);
    ASSERT_TRUE(node1 != NULL);

    rt_node_t node2 = rt_insert(m_rank_tree, 7, 2);
    ASSERT_TRUE(node2 != NULL);

    rt_node_t node3 = rt_insert(m_rank_tree, 2, 3);
    ASSERT_TRUE(node3 != NULL);

    EXPECT_TRUE(rt_next(m_rank_tree, node2) == node1);
    EXPECT_TRUE(rt_next(m_rank_tree, node1) == node3);
    EXPECT_TRUE(rt_next(m_rank_tree, node3) == NULL);

    EXPECT_TRUE(rt_pre(m_rank_tree, node3) == node1);
    EXPECT_TRUE(rt_pre(m_rank_tree, node1) == node2);
    EXPECT_TRUE(rt_pre(m_rank_tree, node2) == NULL);
}
