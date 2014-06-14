#include <assert.h>
#include "cpe/xcalc/xcalc_predicate.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite/ui_sprite_event.h"
#include "ui_sprite_fsm_ins_transition_i.h"
#include "ui_sprite_fsm_ins_state_i.h"

ui_sprite_fsm_transition_t
ui_sprite_fsm_transition_create(
    ui_sprite_fsm_state_t state,
    const char * event, const char * to_state, const char * call_state, const char * condition)
{
    ui_sprite_fsm_module_t module = state->m_ins->m_module;
    ui_sprite_fsm_transition_t transition;
    size_t event_name_len = strlen(event) + 1;
    size_t to_state_len = to_state ? (strlen(to_state) + 1) : 0;
    size_t call_state_len = call_state ? (strlen(call_state) + 1) : 0;

    transition = mem_alloc(
        module->m_alloc,
        sizeof(struct ui_sprite_fsm_transition) + event_name_len + to_state_len + call_state_len);
    if (transition == NULL) {
        CPE_ERROR(module->m_em, "state %s: create transition: alloc fail!", state->m_name);
        return NULL;
    }

    transition->m_state = state;
    transition->m_handler = NULL;
    transition->m_event = (const char *)(transition + 1);
    transition->m_to_state = to_state ? (transition->m_event + event_name_len) : NULL;
    transition->m_call_state = call_state ? (transition->m_event + event_name_len + to_state_len) : NULL;
    transition->m_condition = NULL;

    memcpy((void*)transition->m_event, event, event_name_len);
    if (to_state) memcpy((void*)transition->m_to_state, to_state, to_state_len);
    if (call_state) memcpy((void*)transition->m_call_state, call_state, call_state_len);

    if (condition) {
        transition->m_condition = xpredicate_parse(module->m_alloc, condition, module->m_em);
        if (transition->m_condition == NULL) {
            CPE_ERROR(
                module->m_em, "state %s: create transition: parse condition %s fail!",
                state->m_name, condition);
            mem_free(module->m_alloc, transition);
            return NULL;
        }
    }

    TAILQ_INSERT_TAIL(&state->m_transitions, transition, m_next_for_state);

    return transition;
}

ui_sprite_fsm_transition_t
ui_sprite_fsm_transition_clone(
    ui_sprite_fsm_state_t fsm_state, ui_sprite_fsm_transition_t from)
{
    ui_sprite_fsm_transition_t transition = 
        ui_sprite_fsm_transition_create(
            fsm_state,
            from->m_event, from->m_to_state, from->m_call_state, NULL);

    return transition;
}

void ui_sprite_fsm_transition_free(ui_sprite_fsm_transition_t transition) {
    ui_sprite_fsm_state_t state = transition->m_state;
    ui_sprite_fsm_module_t module = state->m_ins->m_module;

    if (transition->m_handler) {
        ui_sprite_fsm_transition_exit(transition);
        assert(transition->m_handler == NULL);
    }

    if (transition->m_condition) {
        xpredicate_free(module->m_alloc, transition->m_condition);
    }

    TAILQ_REMOVE(&state->m_transitions, transition, m_next_for_state);

    mem_free(module->m_alloc, transition);
}

static void ui_sprite_fsm_transaction_clear_call_stack(ui_sprite_fsm_state_t state) {
    while(state) {
        ui_sprite_fsm_state_t next_state = state->m_return_to;
        state->m_return_to = NULL;
        state = next_state;
    }
}

static void ui_sprite_fsm_transaction_save_event(
    ui_sprite_fsm_module_t module, ui_sprite_fsm_transition_t transition, ui_sprite_fsm_state_t state, ui_sprite_event_t evt)
{
    if (state->m_enter_event) {
        mem_free(module->m_alloc, state->m_enter_event);
    }
    state->m_enter_event = ui_sprite_event_copy(module->m_alloc, evt);

    if (state->m_enter_event == NULL) {
        CPE_ERROR(
            module->m_em, "state %s: transition of %s: clone event fail!",
            state->m_name, transition->m_event);
        return;
    }
}

static void ui_sprite_fsm_transaction_on_event(void * ctx, ui_sprite_event_t evt) {
    ui_sprite_fsm_transition_t transition = ctx;
    ui_sprite_fsm_ins_t fsm = transition->m_state->m_ins;
    ui_sprite_entity_t entity = ui_sprite_fsm_to_entity(fsm);
    ui_sprite_fsm_module_t module = fsm->m_module;
    ui_sprite_fsm_state_t to_state = NULL;
    ui_sprite_fsm_state_t call_state = NULL;

    assert(fsm->m_cur_state == transition->m_state);

    if (transition->m_condition) {
        if (!xpredicate_eval(transition->m_condition)) return;
    }

    if (transition->m_to_state) {
        to_state = ui_sprite_fsm_state_find_by_name(fsm, transition->m_to_state);
        if (to_state == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): %s: transition of %s: to state %s not exist!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_ins_path(fsm),
                transition->m_event, transition->m_to_state);
            return;
        }
    }

    if (transition->m_call_state) {
        call_state = ui_sprite_fsm_state_find_by_name(fsm, transition->m_call_state);
        if (call_state == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): %s: transition of %s: call state %s not exist!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_ins_path(fsm),
                transition->m_event, transition->m_call_state);
            return;
        }
    }

    if (to_state != NULL) {
        ui_sprite_fsm_transaction_clear_call_stack(fsm->m_cur_state);
    }

    if (call_state) {
        ui_sprite_fsm_transaction_save_event(module, transition, call_state, evt);

        call_state->m_return_to = to_state ? to_state : fsm->m_cur_state;

        ui_sprite_fsm_state_exit(fsm->m_cur_state);
        assert(fsm->m_cur_state == NULL);

        if (ui_sprite_fsm_state_enter(call_state) == 0) return;

        CPE_ERROR(
            module->m_em, "entity %d(%s): %s: transition of %s: enter state %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_ins_path(fsm),
            transition->m_event, call_state->m_name);

        if (to_state) {
            if (ui_sprite_fsm_state_enter(to_state) == 0) return;

            CPE_ERROR(
                module->m_em, "entity %d(%s): %s: transition of %s: enter state %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_ins_path(fsm),
                transition->m_event, to_state->m_name);

            return;
        }

        return;
    }
    else if (to_state) {
        ui_sprite_fsm_transaction_save_event(module, transition, to_state, evt);

        ui_sprite_fsm_state_exit(fsm->m_cur_state);
        assert(fsm->m_cur_state == NULL);

        if (ui_sprite_fsm_state_enter(to_state) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): %s: transition of %s: enter state %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_ins_path(fsm),
                transition->m_event, to_state->m_name);
            return;
        }
    }
    else {
        CPE_ERROR(
            module->m_em, "enter %d(%s): %s: transition of %s: no to state or call state!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_ins_path(fsm),
            transition->m_event)
        return;
    }
}

int ui_sprite_fsm_transition_enter(ui_sprite_fsm_transition_t transition) {
    ui_sprite_fsm_state_t state = transition->m_state;
    ui_sprite_fsm_module_t module = state->m_ins->m_module;
    ui_sprite_component_t component = ui_sprite_fsm_to_component(state->m_ins);

    if (transition->m_handler) {
        CPE_ERROR(
            module->m_em, "state %s: transition of %s enter: already active!",
            state->m_name, transition->m_event);
        return -1;
    }

    transition->m_handler =
        ui_sprite_component_add_event_handler(
            component,
            ui_sprite_event_scope_self,
            transition->m_event,
            ui_sprite_fsm_transaction_on_event, transition);
    if (transition->m_handler == NULL) {
        CPE_ERROR(
            module->m_em, "state %s: transition of %s enter: add event handler fail!!",
            state->m_name, transition->m_event);
        return -1;
    }

    return 0;
}

void ui_sprite_fsm_transition_exit(ui_sprite_fsm_transition_t transition) {
    if (transition->m_handler) {

        ui_sprite_event_handler_free(
            ui_sprite_fsm_to_world(transition->m_state->m_ins),
            transition->m_handler);

        transition->m_handler = NULL;
    }
}
