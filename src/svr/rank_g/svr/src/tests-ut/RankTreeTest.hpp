#ifndef SVR_RANK_G_SVR_TEST_RANKTREE_H
#define SVR_RANK_G_SVR_TEST_RANKTREE_H
#include "RankGSvrTest.hpp"
#include "../rank_g_svr_rank_tree.h"


typedef ::Loki::NullType RankTreeTestBase;

class RankTreeTest : public testenv::fixture<RankTreeTestBase, RankGSvrTest> {
public:
    void SetUp();
    void TearDown();

    void t_rank_tree_create(uint32_t node_count);

    const char * t_rank_tree_dump(void);

    rt_t m_rank_tree;
};

#endif
