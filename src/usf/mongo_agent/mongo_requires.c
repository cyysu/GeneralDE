#include "assert.h"
#include "cpe/pal/pal_stdlib.h"
#include "usf/logic/logic_require.h"
#include "usf/logic/logic_manage.h"
#include "usf/mongo_agent/mongo_agent.h"
#include "mongo_internal_ops.h"

static void mongo_agent_check_requires(mongo_agent_t agent) {
    uint32_t i;
    int remove_count = 0;

    for(i = 0; i < agent->m_runing_require_count; ) {
        logic_require_t require = logic_require_find(agent->m_logic_mgr, agent->m_runing_requires[i]);
        if (require == NULL) {
            if (i + 1 < agent->m_runing_require_count) {
                memmove(
                    agent->m_runing_requires + i,
                    agent->m_runing_requires + i + 1,
                    sizeof(logic_require_id_t) * (agent->m_runing_require_count - i - 1));
            }
            --agent->m_runing_require_count;
            ++remove_count;
        }
        else {
            ++i;
        }
    }

    if (agent->m_debug) {
        CPE_INFO(
            agent->m_em, "%s: notify_check_requires: remove %d, remain count %d!",
            mongo_agent_name(agent), remove_count, agent->m_runing_require_count);
    }
}

int mongo_agent_save_require_id(mongo_agent_t agent, logic_require_id_t id) {
    int i;

    if (agent->m_runing_require_op_count >= agent->m_runing_require_check_span) {
        mongo_agent_check_requires(agent);
        agent->m_runing_require_op_count = 0;
    }

    if (agent->m_runing_require_count >= agent->m_runing_require_capacity) {
        uint32_t new_capacity;
        logic_require_id_t * new_buf;
        new_capacity = 
            agent->m_runing_require_capacity < 128 ? 128 : agent->m_runing_require_capacity * 2;
        new_buf = (logic_require_id_t * )mem_alloc(agent->m_alloc, sizeof(logic_require_id_t) * new_capacity);
        if (new_buf == NULL) return -1;

        if (agent->m_runing_requires) {
            memcpy(new_buf, agent->m_runing_requires, sizeof(logic_require_id_t) * agent->m_runing_require_count);
            mem_free(agent->m_alloc, agent->m_runing_requires);
        }

        agent->m_runing_requires = new_buf;
        agent->m_runing_require_capacity = new_capacity;
    }

    assert(agent->m_runing_requires);
    assert(agent->m_runing_require_count < agent->m_runing_require_capacity);

    agent->m_runing_requires[agent->m_runing_require_count] = id;
    ++agent->m_runing_require_count;
    for(i = agent->m_runing_require_count - 1; i > 0; --i) {
        logic_require_id_t buf;
        if (agent->m_runing_requires[i] >= agent->m_runing_requires[i - 1]) break;

        buf = agent->m_runing_requires[i];
        agent->m_runing_requires[i] = agent->m_runing_requires[i - 1];
        agent->m_runing_requires[i - 1] = buf;
    }

    ++agent->m_runing_require_op_count;

    if (agent->m_debug >= 2) {
        CPE_INFO(
            agent->m_em, "%s: mongo_agent_remove_require_id: add require %d at %d, count=%d, op-count=%d!",
            mongo_agent_name(agent), id, i, agent->m_runing_require_count, agent->m_runing_require_op_count);
    }

    return 0;
}

int mongo_agent_require_id_cmp(const void * l, const void * r) {
    logic_require_id_t l_id = *((const logic_require_id_t *)l);
    logic_require_id_t r_id = *((const logic_require_id_t *)r);

    return l_id < r_id ? -1
        : l_id == r_id ? 0
        : 1;
}

int mongo_agent_remove_require_id(mongo_agent_t agent, logic_require_id_t id) {
    logic_require_id_t * found;
    int found_pos;

    if (agent->m_runing_require_count == 0) {
        if (agent->m_debug >= 2) {
            CPE_INFO(
                agent->m_em, "%s: mongo_agent_remove_require_id: remove require %d fail, no any require!",
                mongo_agent_name(agent), id);
        }
        return -1;
    }

    assert(agent->m_runing_requires);

    found =
        (logic_require_id_t *)bsearch(
            &id,
            agent->m_runing_requires,
            agent->m_runing_require_count,
            sizeof(id),
            mongo_agent_require_id_cmp);
    if (!found) {
        if (agent->m_debug >= 2) {
            CPE_INFO(
                agent->m_em, "%s: mongo_agent_remove_require_id: remove require %d fail, not found!",
                mongo_agent_name(agent), id);
        }
        return -1;
    }

    found_pos = found - agent->m_runing_requires;
    assert(found_pos >= 0 && found_pos < (int)agent->m_runing_require_count);

    if (found_pos + 1 < (int)agent->m_runing_require_count) {
        memmove(found, found + 1, sizeof(logic_require_id_t) * (agent->m_runing_require_count - found_pos - 1));
    }

    --agent->m_runing_require_count;

    if (agent->m_debug >= 2) {
        CPE_INFO(
            agent->m_em, "%s: mongo_agent_remove_require_id: remove require %d at %d, left-count=%d!",
            mongo_agent_name(agent), id, found_pos, agent->m_runing_require_count);
    }

    return 0;
}

void mongo_agent_notify_all_require_disconnect(mongo_agent_t agent) {
    uint32_t i;
    int notified_count = 0;

    for(i = 0; i < agent->m_runing_require_count; ++i) {
        logic_require_t require = logic_require_find(agent->m_logic_mgr, agent->m_runing_requires[i]);
        if (require) {
            ++notified_count;
            logic_require_set_error(require);
        }
    }

    agent->m_runing_require_count = 0;

    if (agent->m_debug) {
        CPE_INFO(
            agent->m_em, "%s: notify_all_rquire_disconect: processed %d requires!",
            mongo_agent_name(agent), notified_count);
    }
}


