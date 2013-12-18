#include "cpepp/utils/MemBuffer.hpp"
#include "RankTreeTest.hpp"

void RankTreeTest::SetUp() {
    Base::SetUp();

    t_rank_g_svr_create();

    m_rank_tree = NULL;
}

void RankTreeTest::TearDown() {
    if (m_rank_tree) {
        rt_free(m_rank_tree);
        m_rank_tree = NULL;
    }

    Base::TearDown();
}

void RankTreeTest::t_rank_tree_create(uint32_t node_count) {
    if (m_rank_tree) {
        rt_free(m_rank_tree);
        m_rank_tree = NULL;
    }

    size_t capacity = rt_buff_calc_capacity(node_count);
    void * buff = t_tmp_alloc(capacity);
    ASSERT_TRUE(buff != NULL);
    
    ASSERT_EQ(0, rt_buff_init(t_em(), node_count, buff, capacity));

    m_rank_tree = rt_create(rank_g_svr(), buff, capacity);
    ASSERT_TRUE(m_rank_tree != NULL);
}

const char * RankTreeTest::t_rank_tree_dump(void) {
    Cpe::Utils::MemBuffer buf(t_tmp_allocrator());
    return rt_dump(m_rank_tree, buf);
}

