#include <assert.h>
#include "cpe/utils/stream_buffer.h"
#include "rank_g_svr_ops.h"
#include "rank_g_svr_rank_tree.h"

static rt_node_t rt_node_alloc(rt_t rank_tree, uint32_t value, uint32_t record_id);
static void rt_node_free(rt_t rank_tree, rt_node_t node);
static void rt_node_color_flip(rt_t rank_tree, rt_node_t node);
static rt_node_t rt_node_rotate(rt_t rank_tree, rt_node_t h, int dir);
static rt_node_t rt_node_go(rt_t rank_tree, rt_node_t h, int dir);

#define rt_node_size(__tree, __idx) (__idx == INVALID_RANK_TREE_NODE_POS ? 0 : __tree->m_nodes[__idx].m_size)

#define rt_node_update_size(__tree, __node) \
    __node->m_size = (1 + rt_node_size(__tree, __node->m_child[0]) + rt_node_size(__tree, __node->m_child[1]))

#define rt_node_is_red(__node) (__node && __node->m_color == rt_color_red)
#define rt_node_dir(__tree, __p, __n) (__p->m_child[0] == (__n - __tree->m_nodes) ? 0 : 1)
#define rt_node_get(__tree, __idx) ((__idx) == INVALID_RANK_TREE_NODE_POS ? NULL : __tree->m_nodes + (__idx))
#define rt_node_idx(__tree, __node) (__node ? (__node - __tree->m_nodes) : INVALID_RANK_TREE_NODE_POS)

rt_t rt_create(rank_g_svr_t svr, void * buff, size_t buff_capacity) {
    rt_t tree;
    struct rt_head * head = buff;

    if (rt_buff_calc_capacity(head->m_node_capacity) > buff_capacity) {
        CPE_ERROR(
            svr->m_em, "%s: rank_tree_create: buf capacity not enouth, require %d(node_count=%d), but only %d!",
            rank_g_svr_name(svr),
            (int)rt_buff_calc_capacity(head->m_node_capacity), head->m_node_capacity, (int)buff_capacity);
        return NULL;
    }

    tree = mem_alloc(svr->m_alloc, sizeof(struct rt));
    if (tree == NULL) {
        CPE_ERROR(svr->m_em, "%s: rank_tree_create: create fail!", rank_g_svr_name(svr));
        return NULL;
    }

    tree->m_svr = svr;
    tree->m_head = head;
    tree->m_nodes = (void*)(head + 1);

    return tree;
}

void rt_free(rt_t rank_tree) {
    mem_free(rank_tree->m_svr->m_alloc, rank_tree);
}

rt_node_t rt_get(rt_t rank_tree, uint32_t pos) {
    rt_node_t n = rt_node_get(rank_tree, pos);
    return n;
}

uint32_t rt_idx(rt_t rank_tree, rt_node_t node) {
    return node - rank_tree->m_nodes;
}

int rt_find_by_rank(rt_t rank_tree, uint32_t rank_idx, rt_node_t * node) {
    return 0;
}

rt_node_t rt_pre(rt_t rank_tree, rt_node_t node) {
    return rt_node_go(rank_tree, node, 0);
}

rt_node_t rt_next(rt_t rank_tree, rt_node_t node) {
    return rt_node_go(rank_tree, node, 1);
}

rt_node_t
rt_insert(rt_t rank_tree, uint32_t value, uint32_t record_id) {
    rt_node_t new_node;
    rt_node_t p;
    rt_node_t n;
    rt_node_t root;
    uint32_t * insert_pos;

    new_node = rt_node_alloc(rank_tree, value, record_id);
    if (new_node == NULL) {
        CPE_ERROR(rank_tree->m_svr->m_em, "%s: rank_tree_insert: rank tree is full!", rank_g_svr_name(rank_tree->m_svr));
        return NULL;
    }

    insert_pos = &rank_tree->m_head->m_root;

    /*插入新节点到树中 */
    while((*insert_pos) != INVALID_RANK_TREE_NODE_POS) {
        rt_node_t c = rank_tree->m_nodes + (*insert_pos);
        int dir = !(value > c->m_value);
        new_node->m_parent = *insert_pos;
        insert_pos = &c->m_child[dir];
    }

    assert(insert_pos);
    assert(*insert_pos == INVALID_RANK_TREE_NODE_POS);
    *insert_pos = rt_node_idx(rank_tree, new_node);

    n = new_node;
    p = rt_node_get(rank_tree, n->m_parent);
    while(rt_node_is_red(p)) {
        rt_node_t g;
        rt_node_t u;
        int pdir;

        rt_node_update_size(rank_tree, p);

        g = rt_node_get(rank_tree, p->m_parent);
        assert(g);
        rt_node_update_size(rank_tree, g);

        pdir = rt_node_dir(rank_tree, g, p);

        u = rt_node_get(rank_tree, g->m_child[!pdir]);

        if (!rt_node_is_red(u)) {
            if (rt_node_dir(rank_tree, p, n) != pdir) {
                p = rt_node_rotate(rank_tree, p, pdir);
            }

            g = rt_node_rotate(rank_tree, g, !pdir);
            if (g->m_parent == INVALID_RANK_TREE_NODE_POS) {
                rank_tree->m_head->m_root = g - rank_tree->m_nodes;
            }

            break;
        }

        rt_node_color_flip(rank_tree, g);
        n = g;
        p = rt_node_get(rank_tree, n->m_parent);
    }

    root = rt_node_get(rank_tree, rank_tree->m_head->m_root);
    assert(root);
    root->m_color = rt_color_black;

    while(p) {
        rt_node_update_size(rank_tree, p);
        p = rt_node_get(rank_tree, p->m_parent);
    }

    return new_node;
}

void rt_erase(rt_t rank_tree, rt_node_t n) {
    rt_node_t c;
    rt_node_t p;
    rt_node_t saved_n;
    rt_node_t saved_p;
    rt_node_t root;
    uint32_t tmp;
    assert(rank_tree->m_head->m_root != INVALID_RANK_TREE_NODE_POS);

    if (n->m_child[0] != INVALID_RANK_TREE_NODE_POS && n->m_child[1] != INVALID_RANK_TREE_NODE_POS) {
        rt_node_t successor = rt_next(rank_tree, n);

        if (rt_node_idx(rank_tree, successor) == n->m_child[1]) {
            rt_node_t g;

            n->m_child[1] = successor->m_child[1];    // n child1 set
            c = rt_node_get(rank_tree, n->m_child[1]);
            if (c) {
                c->m_parent = rt_node_idx(rank_tree, n);                          // n child1 fix
            }
            successor->m_child[1] = rt_node_idx(rank_tree, n);                 // successor child1 set && n parent fix.
            successor->m_parent = n->m_parent;              // successor parent set
            n->m_parent = rt_node_idx(rank_tree, successor);                      // n parent set && successor child1 fix.

            g = rt_node_get(rank_tree, successor->m_parent);
            if (g) {
                g->m_child[rt_node_dir(rank_tree, g, n)] = rt_node_idx(rank_tree, successor);// successor parent fix.
            }
            // n child1 done, n parent done, successor child1 done, successor parent done.
        }
        else {
            tmp = n->m_parent, n->m_parent = successor->m_parent, successor->m_parent = tmp;

            if ((c = rt_node_get(rank_tree, n->m_parent))) {
                c->m_child[rt_node_dir(rank_tree, c, successor)] = rt_node_idx(rank_tree, n); // n parent fix.
            }

            if ((c = rt_node_get(rank_tree, successor->m_parent))) {
                c->m_child[rt_node_dir(rank_tree, c, n)] = rt_node_idx(rank_tree, successor);// successor parent fix.
            }

            tmp = n->m_child[1], n->m_child[1] = successor->m_child[1], successor->m_child[1] = tmp; // n child1 set && successor set.

            if ((c = rt_node_get(rank_tree, n->m_child[1]))) {
                c->m_parent = rt_node_idx(rank_tree, n);                               // n child1 fix.
            }

            if ((c = rt_node_get(rank_tree, successor->m_child[1]))) {
                c->m_parent = rt_node_idx(rank_tree, successor);                      // successor child1 fix.
            }
            // n child1 done, n parent done, successor child1 done, successor parent done.
        }

        tmp = n->m_child[0], n->m_child[0] = successor->m_child[0], successor->m_child[0] = tmp; // n child0 set && successor child0 set.

        c = rt_node_get(rank_tree, n->m_child[0]);
        if (c) {
            c->m_parent = rt_node_idx(rank_tree, n);                                  // n child0 fix.
        }

        c = rt_node_get(rank_tree, successor->m_child[0]);
        if (c) {
            c->m_parent = rt_node_idx(rank_tree, successor);                          // successor fix.
        }

        tmp = n->m_color, n->m_color = successor->m_color, successor->m_color = tmp;
        tmp = n->m_size, n->m_size = successor->m_size, successor->m_size = tmp;

        if (successor->m_parent == INVALID_RANK_TREE_NODE_POS) {
            rank_tree->m_head->m_root = rt_node_idx(rank_tree, successor);
        }
    }

    saved_n = n;
    p = rt_node_get(rank_tree, n->m_parent);
    c = rt_node_get(rank_tree, n->m_child[0]);
    if (c == NULL) c = rt_node_get(rank_tree, n->m_child[1]);

    if (c) {
        c->m_parent = rt_node_idx(rank_tree, p);
        if (rt_node_is_red(c)) {
            c->m_color = rt_color_black;
            goto finished;
        }
    }

    if (p == NULL || rt_node_is_red(n)) {
        goto finished;
    }
        
    n->m_size = 0;
    for (;
         !rt_node_is_red(n) && p;
         n = p, p = rt_node_get(rank_tree, n->m_parent))
    {
        int dir;
        rt_node_t sibling;
        rt_node_t t1;
        rt_node_t t2;

        rt_node_update_size(rank_tree, p);
        dir = rt_node_dir(rank_tree, p, n);

        sibling = rt_node_get(rank_tree, p->m_child[!dir]);
        if (rt_node_is_red(sibling)) {
            p = rt_node_rotate(rank_tree, p, dir);
            if (p->m_parent == INVALID_RANK_TREE_NODE_POS) {
                rank_tree->m_head->m_root = rt_node_idx(rank_tree, p);
            }

            p = rt_node_get(rank_tree, n->m_parent);
            dir = rt_node_dir(rank_tree, p, n);
            sibling = rt_node_get(rank_tree, p->m_child[!dir]);
        }

        t1 = rt_node_get(rank_tree, sibling->m_child[dir]);
        t2 = rt_node_get(rank_tree, sibling->m_child[!dir]);
        if(rt_node_is_red(t1) && !rt_node_is_red(t2)) {
            sibling = rt_node_rotate(rank_tree, sibling, !dir);
        }

        t1 = rt_node_get(rank_tree, sibling->m_child[!dir]);
        if (rt_node_is_red(t1)) {
            t1->m_color = rt_color_black;
            p = rt_node_rotate(rank_tree, p, dir);
            if (p->m_parent == INVALID_RANK_TREE_NODE_POS) {
                rank_tree->m_head->m_root = rt_node_idx(rank_tree, p);
            }
            break;
        }

        sibling->m_color = rt_color_red;
    }

    if (rt_node_is_red(n)) {
        n->m_color = rt_color_black;
    }

finished:
    saved_p = rt_node_get(rank_tree, saved_n->m_parent);
    if (saved_p) {
        saved_p->m_child[rt_node_dir(rank_tree, saved_p, saved_n)] = rt_node_idx(rank_tree, c);
    }
    else {
        rank_tree->m_head->m_root = rt_node_idx(rank_tree, c);
    }

    if ((root = rt_node_get(rank_tree, rank_tree->m_head->m_root))) {
        root->m_color = rt_color_black;
    }

    while(p) {
        rt_node_update_size(rank_tree, p);
        p = rt_node_get(rank_tree, p->m_parent);
    }

    rt_node_free(rank_tree, saved_n);
}

uint32_t rt_size(rt_t rank_tree) {
    return rank_tree->m_head->m_root == INVALID_RANK_TREE_NODE_POS
        ? 0
        : rank_tree->m_nodes[rank_tree->m_head->m_root].m_size;
}

uint32_t rt_capacity(rt_t rank_tree) {
    return rank_tree->m_head->m_node_capacity;
}

rt_node_t rt_last(rt_t rank_tree) {
    rt_node_t n;

    if (rank_tree->m_head->m_root == INVALID_RANK_TREE_NODE_POS) return NULL;

    for(n = rank_tree->m_nodes + rank_tree->m_head->m_root;
        n->m_child[1] != INVALID_RANK_TREE_NODE_POS;
        n = rank_tree->m_nodes + n->m_child[1])
    {
    }

    return n;
}

rt_node_t rt_first(rt_t rank_tree) {
    rt_node_t n;

    if (rank_tree->m_head->m_root == INVALID_RANK_TREE_NODE_POS) return NULL;

    for(n = rank_tree->m_nodes + rank_tree->m_head->m_root;
        n->m_child[0] != INVALID_RANK_TREE_NODE_POS;
        n = rank_tree->m_nodes + n->m_child[0])
    {
    }

    return n;
}

uint32_t rt_node_value(rt_node_t node) {
    return node->m_value;
}

uint32_t rt_node_record_id(rt_node_t node) {
    return node->m_record_id;
}

size_t rt_buff_calc_capacity(uint32_t node_count) {
    return sizeof(struct rt_head) + sizeof(struct rt_node) * node_count;
}

int rt_buff_init(error_monitor_t em, uint32_t node_count, void * buff, size_t buff_capacity) {
    struct rt_head * head;
    rt_node_t nodes;
    uint32_t i;
    uint32_t * free_node;

    assert(buff);

    if (rt_buff_calc_capacity(node_count) > buff_capacity) {
        CPE_ERROR(
            em, "rt_buff_init: buf capacity not enouth, require %d(node_count=%d), but only %d!",
            (int)rt_buff_calc_capacity(node_count), node_count, (int)buff_capacity);
        return -1;
    }

    bzero(buff, buff_capacity);

    head = buff;
    nodes = (void*)(head + 1);

    head->m_node_capacity = node_count;
    head->m_root = INVALID_RANK_TREE_NODE_POS;

    free_node = &head->m_free_node;
    for(i = 0; i < node_count; ++i) {
        *free_node = i;
        free_node = &nodes[i].m_parent;
    }

    *free_node = INVALID_RANK_TREE_NODE_POS;

    return 0;
}

static rt_node_t
rt_node_alloc(rt_t rank_tree, uint32_t value, uint32_t record_id) {
    rt_node_t r;

    if (rank_tree->m_head->m_free_node == INVALID_RANK_TREE_NODE_POS) return NULL;

    r = rank_tree->m_nodes + rank_tree->m_head->m_free_node;
    rank_tree->m_head->m_free_node = r->m_parent;
    r->m_parent = r->m_child[0] = r->m_child[1] = INVALID_RANK_TREE_NODE_POS;
    r->m_value = value;
    r->m_record_id = record_id;
    r->m_size = 1;
    r->m_color = rt_color_red;

    return r; 
}

static void rt_node_free(rt_t rank_tree, rt_node_t node) {
    node->m_parent = rank_tree->m_head->m_free_node;
    node->m_color = 0;
    rank_tree->m_head->m_free_node = node - rank_tree->m_nodes;
    assert(rank_tree->m_head->m_free_node >= 0 && rank_tree->m_head->m_free_node < rank_tree->m_head->m_node_capacity);
}

static void rt_node_color_flip(rt_t rank_tree, rt_node_t node) {
    int i;

    node->m_color = !node->m_color;

    for(i = 0; i < 2; ++i) {
        rt_node_t c = rt_node_get(rank_tree, node->m_child[i]);
        assert(c);
        c->m_color = !c->m_color;
    }
}

static rt_node_t 
rt_node_rotate(rt_t rank_tree, rt_node_t h, int dir) {
    int opp_dir;
    rt_node_t x;
    rt_node_t p;
    uint32_t tmp;

    opp_dir = !dir;
    x = rt_node_get(rank_tree, h->m_child[opp_dir]);

    h->m_child[opp_dir] = x->m_child[dir];
    x->m_child[dir] = h - rank_tree->m_nodes;

    tmp = x->m_color;
    x->m_color = h->m_color;
    h->m_color = tmp;

    p = rt_node_get(rank_tree, h->m_parent);
    if (p) {
        int pdir = rt_node_dir(rank_tree, p, h);
        p->m_child[pdir] = x - rank_tree->m_nodes;
    }

    x->m_parent = h->m_parent;
    h->m_parent = x - rank_tree->m_nodes;

    if (h->m_child[opp_dir] != INVALID_RANK_TREE_NODE_POS) {
        rt_node_get(rank_tree, h->m_child[opp_dir])->m_parent = h - rank_tree->m_nodes;
    }

    rt_node_update_size(rank_tree, h);
    rt_node_update_size(rank_tree, x);

    return x;
}

static rt_node_t 
rt_node_go(rt_t rank_tree, rt_node_t h, int dir) {
    int opp_dir = !dir;

    assert(h);

    if (h->m_child[dir] != INVALID_RANK_TREE_NODE_POS) {
        h = rank_tree->m_nodes + h->m_child[dir];
        while(h->m_child[dir] != INVALID_RANK_TREE_NODE_POS) {
            h = rank_tree->m_nodes + h->m_child[dir];
        }
        return h;
    }
    else {
        rt_node_t p = rt_node_get(rank_tree, h->m_parent);
        while(p) {
            if (rt_node_dir(rank_tree, p, h) == opp_dir) {
                return p;
            }

            h = p;
            p = rt_node_get(rank_tree, h->m_parent);
        }
        return NULL;
    }
}

static void rt_print(
    write_stream_t s, rt_t rank_tree, rt_node_t p, rt_node_t node, int ident)
{
    int i;

    stream_putc_count(s, ' ', ident);
    stream_printf(s, "%d (%s)\n", node->m_value, (node->m_color == rt_color_red ? "red" : "black"));

    for(i = 0; i < 2; ++i) {
        rt_node_t c = rt_node_get(rank_tree, node->m_child[i]);
        if (c) rt_print(s, rank_tree, node, c, ident + 4);
    }
}

const char * rt_dump(rt_t rank_tree, mem_buffer_t buffer) {
    struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(buffer);
    rt_node_t root;    

    root = rt_node_get(rank_tree, rank_tree->m_head->m_root);
    if (root) {
        rt_print((write_stream_t)&stream, rank_tree, NULL, root, 0);
    }

    stream_putc((write_stream_t)&stream, 0);

    return mem_buffer_make_continuous(buffer, 0);
}
