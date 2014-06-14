#include <assert.h>
#include "ui/sprite/ui_sprite_entity.h"
#include "ui_sprite_fsm_ins_state_i.h"
#include "ui_sprite_fsm_ins_action_i.h"
#include "ui_sprite_fsm_ins_transition_i.h"
#include "ui_sprite_fsm_action_meta_i.h"

static ui_sprite_fsm_state_t ui_sprite_fsm_state_create_i(ui_sprite_fsm_ins_t fsm, const char * name) {
    ui_sprite_fsm_module_t module = fsm->m_module;
    ui_sprite_fsm_state_t fsm_state;
    size_t name_len = strlen(name) + 1;

    fsm_state = mem_alloc(module->m_alloc, sizeof(struct ui_sprite_fsm_state) + name_len);
    if (fsm_state == NULL) {
        CPE_ERROR(module->m_em, "fsm create state %s: alloc fail", name);
        return NULL;
    }

    fsm_state->m_ins = fsm;
    fsm_state->m_id = fsm->m_max_state_id + 1;
    fsm_state->m_name = (const char *)(fsm_state + 1);
    fsm_state->m_return_to = NULL;
    fsm_state->m_enter_event = NULL;

    memcpy(fsm_state + 1, name, name_len);

    TAILQ_INIT(&fsm_state->m_transitions);
    TAILQ_INIT(&fsm_state->m_actions);
    TAILQ_INIT(&fsm_state->m_updating_actions);
    TAILQ_INIT(&fsm_state->m_waiting_actions);
    TAILQ_INIT(&fsm_state->m_runing_actions);
    TAILQ_INIT(&fsm_state->m_done_actions);

    TAILQ_INSERT_TAIL(&fsm->m_states, fsm_state, m_next_for_ins);

    return fsm_state;
}

ui_sprite_fsm_state_t ui_sprite_fsm_state_create(ui_sprite_fsm_ins_t fsm, const char * name) {
    ui_sprite_fsm_state_t fsm_state = ui_sprite_fsm_state_create_i(fsm, name);

    if (fsm_state) fsm->m_max_state_id++;

    return fsm_state;
}

ui_sprite_fsm_state_t ui_sprite_fsm_state_clone(ui_sprite_fsm_ins_t fsm, ui_sprite_fsm_state_t from) {
    ui_sprite_fsm_state_t to = ui_sprite_fsm_state_create_i(fsm, from->m_name);
    ui_sprite_fsm_action_t from_fsm_action;
    ui_sprite_fsm_transition_t from_fsm_transition;

    if (to == NULL) return NULL;


    fsm->m_max_state_id++;

    TAILQ_FOREACH(from_fsm_action, &from->m_actions, m_next_for_state) {
        ui_sprite_fsm_action_t to_fsm_action = ui_sprite_fsm_action_clone(to, from_fsm_action);
        if (to_fsm_action == NULL) {
            ui_sprite_fsm_state_free(to);
            return NULL;
        }
    }

    TAILQ_FOREACH(from_fsm_transition, &from->m_transitions, m_next_for_state) {
        ui_sprite_fsm_transition_t to_fsm_transition = ui_sprite_fsm_transition_clone(to, from_fsm_transition);
        if (to_fsm_transition == NULL) {
            ui_sprite_fsm_state_free(to);
            return NULL;
        }
    }

    return to;
}

void ui_sprite_fsm_state_free(ui_sprite_fsm_state_t fsm_state) {
    ui_sprite_fsm_ins_t fsm = fsm_state->m_ins;
    ui_sprite_fsm_module_t module = fsm->m_module;

    if (fsm->m_cur_state == fsm_state) {
        ui_sprite_fsm_state_exit(fsm_state);
        assert(fsm->m_cur_state == NULL);
    }

    assert(fsm_state->m_enter_event == NULL);

    while(!TAILQ_EMPTY(&fsm_state->m_transitions)) {
        ui_sprite_fsm_transition_free(TAILQ_FIRST(&fsm_state->m_transitions));
    }

    while(!TAILQ_EMPTY(&fsm_state->m_actions)) {
        ui_sprite_fsm_action_free(TAILQ_FIRST(&fsm_state->m_actions));
    }
    assert(TAILQ_EMPTY(&fsm_state->m_updating_actions));
    assert(TAILQ_EMPTY(&fsm_state->m_waiting_actions));
    assert(TAILQ_EMPTY(&fsm_state->m_runing_actions));
    assert(TAILQ_EMPTY(&fsm_state->m_done_actions));

    TAILQ_REMOVE(&fsm->m_states, fsm_state, m_next_for_ins);

    if (fsm->m_init_state == fsm_state) fsm->m_init_state = NULL;
    if (fsm->m_init_call_state == fsm_state) fsm->m_init_call_state = NULL;

    mem_free(module->m_alloc, fsm_state);
}

ui_sprite_fsm_state_t ui_sprite_fsm_state_find_by_name(ui_sprite_fsm_ins_t fsm, const char * name) {
    ui_sprite_fsm_state_t fsm_state;

    TAILQ_FOREACH(fsm_state, &fsm->m_states, m_next_for_ins) {
        if (strcmp(fsm_state->m_name, name) == 0) return fsm_state;
    }

    return NULL;
}

ui_sprite_fsm_state_t ui_sprite_fsm_state_find_by_id(ui_sprite_fsm_ins_t fsm, uint16_t id) {
    ui_sprite_fsm_state_t fsm_state;

    TAILQ_FOREACH(fsm_state, &fsm->m_states, m_next_for_ins) {
        if (fsm_state->m_id == id) return fsm_state;
    }

    return NULL;
}

ui_sprite_fsm_ins_t ui_sprite_fsm_state_fsm(ui_sprite_fsm_state_t fsm_state) {
    return fsm_state->m_ins;
}

uint16_t ui_sprite_fsm_state_id(ui_sprite_fsm_state_t fsm_state) {
    return fsm_state->m_id;
}

const char * ui_sprite_fsm_state_name(ui_sprite_fsm_state_t fsm_state) {
    return fsm_state->m_name;
}

int ui_sprite_fsm_state_enter(ui_sprite_fsm_state_t fsm_state) {
    ui_sprite_fsm_ins_t fsm = fsm_state->m_ins;
    ui_sprite_fsm_module_t module = fsm->m_module;
    ui_sprite_fsm_action_t fsm_action;
    ui_sprite_fsm_transition_t fsm_transition;
    ui_sprite_entity_t entity = ui_sprite_fsm_to_entity(fsm);

    assert(fsm->m_cur_state == NULL);

    fsm->m_cur_state = fsm_state;

    if (ui_sprite_entity_debug(entity)) {
        CPE_INFO(
            module->m_em, "entity %d(%s): %s: enter", 
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_ins_path(fsm));
    }

    TAILQ_FOREACH(fsm_action, &fsm_state->m_actions, m_next_for_state) {
        assert(fsm_action->m_runing_state == ui_sprite_fsm_action_state_deactive);

        if (fsm_action->m_follow_to != NULL) {
            ui_sprite_fsm_action_set_runing_state(fsm_action, ui_sprite_fsm_action_state_waiting);
        }
        else {
            if (ui_sprite_fsm_action_enter(fsm_action) != 0) {
                ui_sprite_fsm_action_set_runing_state(fsm_action, ui_sprite_fsm_action_state_done);
                continue;
            }

            if (ui_sprite_fsm_action_check_do_enter(fsm_action) != 0) {
                ui_sprite_fsm_action_exit(fsm_action);
                continue;
            }
        }
    }

    TAILQ_FOREACH(fsm_transition, &fsm_state->m_transitions, m_next_for_state) {
        if (ui_sprite_fsm_transition_enter(fsm_transition) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): %s: enter fail", 
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_ins_path(fsm));
            goto STATE_ENTER_FAIL;
        }
    }

    return 0;

STATE_ENTER_FAIL:
    TAILQ_FOREACH(fsm_transition, &fsm_state->m_transitions, m_next_for_state) {
        if(fsm_transition->m_handler) {
            ui_sprite_fsm_transition_exit(fsm_transition);
        }
    }

    while(!TAILQ_EMPTY(&fsm_state->m_runing_actions)) {
        ui_sprite_fsm_action_exit(TAILQ_FIRST(&fsm_state->m_runing_actions));
    }

    TAILQ_FOREACH(fsm_action, &fsm_state->m_actions, m_next_for_state) {
        if (fsm_action->m_runing_state != ui_sprite_fsm_action_state_deactive) {
            ui_sprite_fsm_action_set_runing_state(fsm_action, ui_sprite_fsm_action_state_deactive);
        }
    }

    fsm->m_cur_state = NULL;
    return -1;

    assert(TAILQ_EMPTY(&fsm_state->m_runing_actions));
    assert(TAILQ_EMPTY(&fsm_state->m_waiting_actions));
    assert(TAILQ_EMPTY(&fsm_state->m_updating_actions));
    assert(TAILQ_EMPTY(&fsm_state->m_done_actions));
}

void ui_sprite_fsm_state_exit(ui_sprite_fsm_state_t fsm_state) {
    ui_sprite_fsm_action_t fsm_action;
    ui_sprite_fsm_ins_t fsm = fsm_state->m_ins;
    ui_sprite_fsm_module_t module = fsm->m_module;
    ui_sprite_fsm_transition_t fsm_transition;
    ui_sprite_entity_t entity = ui_sprite_fsm_to_entity(fsm);

    assert(fsm->m_cur_state == fsm_state);

    if (ui_sprite_entity_debug(entity)) {
        CPE_INFO(
            module->m_em, "entity %d(%s): %s: exit",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_ins_path(fsm));
    }

    TAILQ_FOREACH(fsm_transition, &fsm_state->m_transitions, m_next_for_state) {
        ui_sprite_fsm_transition_exit(fsm_transition);
    }

    while(!TAILQ_EMPTY(&fsm_state->m_runing_actions)) {
        ui_sprite_fsm_action_exit(TAILQ_FIRST(&fsm_state->m_runing_actions));
    }

    TAILQ_FOREACH_REVERSE(fsm_action, &fsm_state->m_actions, ui_sprite_fsm_action_list, m_next_for_state) {
        if (fsm_action->m_runing_state != ui_sprite_fsm_action_state_deactive) {
            ui_sprite_fsm_action_set_runing_state(fsm_action, ui_sprite_fsm_action_state_deactive);
        }
    }

    if (fsm_state->m_enter_event) {
        mem_free(module->m_alloc, fsm_state->m_enter_event);
        fsm_state->m_enter_event = NULL;
    }

    fsm->m_cur_state = NULL;

    assert(TAILQ_EMPTY(&fsm_state->m_runing_actions));
    assert(TAILQ_EMPTY(&fsm_state->m_waiting_actions));
    assert(TAILQ_EMPTY(&fsm_state->m_updating_actions));
    assert(TAILQ_EMPTY(&fsm_state->m_done_actions));
}

void ui_sprite_fsm_state_update(ui_sprite_fsm_state_t fsm_state, float delta) {
    ui_sprite_fsm_action_t action;
    uint16_t life_circle_action_count = 0;
    ui_sprite_fsm_ins_t fsm = fsm_state->m_ins;
    ui_sprite_fsm_module_t module = fsm->m_module;
    ui_sprite_entity_t entity = ui_sprite_fsm_to_entity(fsm);

    /*所有需要更新的action先更新 */
    for(action = TAILQ_FIRST(&fsm_state->m_updating_actions);
        action != TAILQ_END(&fsm_state->m_updating_actions);
        )
    {
        ui_sprite_fsm_action_t next = TAILQ_NEXT(action, m_next_for_update);

        assert(action->m_meta->m_update_fun);
        action->m_meta->m_update_fun(action, action->m_meta->m_update_fun_ctx, delta);

        action = next;
    }

    /*检查已经完成的action，退出 */
    for(action = TAILQ_FIRST(&fsm_state->m_runing_actions);
        action != TAILQ_END(&fsm_state->m_runing_actions);
        )
    {
        ui_sprite_fsm_action_t next = TAILQ_NEXT(action, m_next_for_work);

        action->m_runing_time += delta;
        switch(action->m_life_circle) {
        case ui_sprite_fsm_action_life_circle_passive:
            break;
        case ui_sprite_fsm_action_life_circle_working:
            if (action->m_is_update) {
                life_circle_action_count++;
            }
            else {
                ui_sprite_fsm_action_exit(action);
            }
            break;
        case ui_sprite_fsm_action_life_circle_endless:
            life_circle_action_count++;
            break;
        case ui_sprite_fsm_action_life_circle_duration:
            if (action->m_runing_time < action->m_duration) {
                life_circle_action_count++;
            }
            else {
                ui_sprite_fsm_action_exit(action);
            }
            break;
        default:
            CPE_ERROR(
                module->m_em, "entity %d(%s): %s: action %s(%s): life-circle %d unknown",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_ins_path(fsm),
                action->m_meta->m_name, action->m_name, action->m_life_circle);
            ui_sprite_fsm_action_exit(action);
            break;
        }

        action = next;
    }

    /*检查所有刚刚完成的action，有follow则启动 */
    while(!TAILQ_EMPTY(&fsm_state->m_done_actions)) {
        ui_sprite_fsm_action_t follow_action;

        action = TAILQ_FIRST(&fsm_state->m_done_actions);
        ui_sprite_fsm_action_set_runing_state(action, ui_sprite_fsm_action_state_deactive);

        TAILQ_FOREACH(follow_action, &action->m_followers, m_next_for_follow) {
            if (follow_action->m_runing_state != ui_sprite_fsm_action_state_waiting) continue;

            if (ui_sprite_fsm_action_enter(follow_action) != 0) {
                ui_sprite_fsm_action_set_runing_state(follow_action, ui_sprite_fsm_action_state_done);
                continue;
            }

            if (ui_sprite_fsm_action_check_do_enter(follow_action) != 0) {
                ui_sprite_fsm_action_exit(follow_action);
                continue;
            }

            life_circle_action_count++;
        }
    }

    if (life_circle_action_count != 0) return;

    /*没有活动的action了，则退出state */
    if (fsm_state->m_return_to && fsm_state->m_return_to->m_enter_event == NULL) {
        fsm_state->m_return_to->m_enter_event = fsm_state->m_enter_event;
        fsm_state->m_enter_event = NULL;
    }
    ui_sprite_fsm_state_exit(fsm_state);

    /*回退调用堆栈 */
    while(fsm_state->m_return_to) {
        ui_sprite_fsm_state_t return_state = fsm_state->m_return_to;
        fsm_state->m_return_to = NULL;
        fsm_state = return_state;
 
        if (ui_sprite_fsm_state_enter(fsm_state) == 0) return;

        if (fsm_state->m_return_to && fsm_state->m_return_to->m_enter_event) {
            fsm_state->m_return_to->m_enter_event = fsm_state->m_enter_event;
            fsm_state->m_enter_event = NULL;
        }
        else {
            mem_free(module->m_alloc, fsm_state->m_enter_event);
            fsm_state->m_enter_event = NULL;
        }
    }

    assert(fsm->m_cur_state == NULL);
    if (fsm->m_parent) {
        TAILQ_REMOVE(&fsm->m_parent->m_childs, fsm, m_next_for_parent);
    }
}
