#include <assert.h>
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/tl/tl_manage.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "ui/sprite/ui_sprite_repository.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_module.h"
#include "ui/sprite_anim/ui_sprite_anim_camera.h"
#include "ui_sprite_touch_mgr_i.h"
#include "ui_sprite_touch_touchable_i.h"
#include "ui_sprite_touch_move_i.h"
#include "ui_sprite_touch_click_i.h"
#include "ui_sprite_touch_trace_i.h"

extern char g_metalib_ui_sprite_touch[];
static void ui_sprite_touch_mgr_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_ui_sprite_touch_mgr = {
    "ui_sprite_touch_mgr",
    ui_sprite_touch_mgr_clear
};

static struct {
    const char * name; 
    int (*init)(ui_sprite_touch_mgr_t mgr);
    void (*fini)(ui_sprite_touch_mgr_t mgr);
} s_auto_reg_products[] = {
    { "Touchable", ui_sprite_touch_touchable_regist, ui_sprite_touch_touchable_unregist }
    , { "touch-move", ui_sprite_touch_move_regist, ui_sprite_touch_move_unregist }
    , { "touch-click", ui_sprite_touch_click_regist, ui_sprite_touch_click_unregist }
};

#define UI_SPRITE_TOUCH_MGR_LOAD_META(__arg, __name) \
    module-> __arg  = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_ui_sprite_touch, __name); \
    assert(module-> __arg)


ui_sprite_touch_mgr_t
ui_sprite_touch_mgr_create(
    gd_app_context_t app, ui_sprite_repository_t repo, ui_sprite_fsm_module_t fsm_module,
    mem_allocrator_t alloc, const char * name, error_monitor_t em)
{
    struct ui_sprite_touch_mgr * module;
    nm_node_t module_node;
    int component_pos;

    assert(app);

    module_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct ui_sprite_touch_mgr));
    if (module_node == NULL) return NULL;

    module = (ui_sprite_touch_mgr_t)nm_node_data(module_node);

    module->m_app = app;
    module->m_alloc = alloc;
    module->m_em = em;
    module->m_repo = repo;
    module->m_fsm = fsm_module;
    module->m_debug = 0;
    module->m_dft_threshold = 3;

    UI_SPRITE_TOUCH_MGR_LOAD_META(m_meta_move_state, "ui_sprite_touch_move_state");
    //UI_SPRITE_TOUCH_MGR_LOAD_META(m_meta_scale_state, "ui_sprite_touch_scale_state");

    TAILQ_INIT(&module->m_touch_traces);
    TAILQ_INIT(&module->m_active_responsers);
    TAILQ_INIT(&module->m_waiting_responsers);

    for(component_pos = 0; component_pos < CPE_ARRAY_SIZE(s_auto_reg_products); ++component_pos) {
        if (s_auto_reg_products[component_pos].init(module) != 0) {
            CPE_ERROR(em, "%s: regist product %s fail!", name, s_auto_reg_products[component_pos].name);

            for(; component_pos > 0; component_pos--) {
                s_auto_reg_products[component_pos - 1].fini(module);
            }

            nm_node_from_data(module_node);
            return NULL;
        }
    }

    nm_node_set_type(module_node, &s_nm_node_type_ui_sprite_touch_mgr);
    return module;
}

static void ui_sprite_touch_mgr_clear(nm_node_t node) {
    ui_sprite_touch_mgr_t module;
    int component_pos;

    module = (ui_sprite_touch_mgr_t)nm_node_data(node);

    for(component_pos = CPE_ARRAY_SIZE(s_auto_reg_products); component_pos > 0; component_pos--) {
        s_auto_reg_products[component_pos - 1].fini(module);
    }

    ui_sprite_touch_touchable_unregist(module);

    assert(TAILQ_EMPTY(&module->m_active_responsers));
    assert(TAILQ_EMPTY(&module->m_waiting_responsers));

    ui_sprite_touch_trace_free_all(module);
}

gd_app_context_t ui_sprite_touch_mgr_app(ui_sprite_touch_mgr_t module) {
    return module->m_app;
}

void ui_sprite_touch_mgr_free(ui_sprite_touch_mgr_t module) {
    nm_node_t module_node;
    assert(module);

	module_node = nm_node_from_data(module);
    if (nm_node_type(module_node) != &s_nm_node_type_ui_sprite_touch_mgr) return;
    nm_node_free(module_node);
}

ui_sprite_touch_mgr_t
ui_sprite_touch_mgr_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_ui_sprite_touch_mgr) return NULL;
    return (ui_sprite_touch_mgr_t)nm_node_data(node);
}

ui_sprite_touch_mgr_t
ui_sprite_touch_mgr_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

	if (name == NULL) name = "ui_sprite_touch_mgr";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_ui_sprite_touch_mgr) return NULL;
    return (ui_sprite_touch_mgr_t)nm_node_data(node);
}

const char * ui_sprite_touch_mgr_name(ui_sprite_touch_mgr_t module) {
    return nm_node_name(nm_node_from_data(module));
}

ui_sprite_touch_responser_t ui_sprite_touch_mgr_find_grab_active_responser(ui_sprite_touch_mgr_t mgr) {
    ui_sprite_touch_responser_t responser;

    TAILQ_FOREACH(responser, &mgr->m_active_responsers, m_next_for_mgr) {
        if (responser->m_is_grab && responser->m_is_start) return responser;
    }

    return NULL;
}

static UI_SPRITE_2D_PAIR
ui_sprite_touch_logic_to_world(ui_sprite_touch_touchable_t touchable, UI_SPRITE_2D_PAIR screen_pt) {
    ui_sprite_entity_t entity;
    ui_sprite_anim_camera_t camera;

    entity = ui_sprite_component_entity(ui_sprite_component_from_data(touchable));

    camera = ui_sprite_anim_camera_find(ui_sprite_entity_world(entity));
    if (camera) {
        return ui_sprite_anim_camera_screen_to_world(camera, screen_pt);
    }
    else {
        return screen_pt;
    }
}

static void ui_sprite_touch_mgr_cancel_other_active(ui_sprite_touch_mgr_t mgr, ui_sprite_touch_responser_t responser) {
    ui_sprite_touch_responser_t check_responser;
    for(check_responser = TAILQ_FIRST(&mgr->m_active_responsers);
        check_responser != TAILQ_END(&mgr->m_active_responsers);
        )
    {
        ui_sprite_touch_responser_t next = TAILQ_NEXT(check_responser, m_next_for_mgr);

        if (check_responser != responser) {
            ui_sprite_touch_responser_cancel(check_responser);
        }

        check_responser = next;
    }

    assert(TAILQ_FIRST(&mgr->m_active_responsers) == responser);
    assert(TAILQ_NEXT(responser, m_next_for_mgr) == TAILQ_END(&mgr->m_active_responsers));
}

static void ui_sprite_touch_process_trace_begin(ui_sprite_touch_mgr_t mgr, ui_sprite_touch_trace_t trace, UI_SPRITE_2D_PAIR screen_pt) {
    ui_sprite_touch_responser_t  responser;

    if ((responser = ui_sprite_touch_mgr_find_grab_active_responser(mgr))) {
        return;
    }

    for(responser = TAILQ_FIRST(&mgr->m_waiting_responsers);
        responser != TAILQ_END(&mgr->m_waiting_responsers);
        )
    {
        ui_sprite_touch_responser_t next = TAILQ_NEXT(responser, m_next_for_mgr);
        UI_SPRITE_2D_PAIR world_pt = ui_sprite_touch_logic_to_world(responser->m_touchable, screen_pt);

        if (ui_sprite_touch_touchable_is_point_in(responser->m_touchable, world_pt)) {
            uint8_t binding_pos;
            ui_sprite_touch_responser_binding_t binding;

            binding_pos = ui_sprite_touch_responser_bind_tracer(responser, trace);

            binding = &responser->m_bindings[binding_pos];

            binding->m_start_screen_pt = screen_pt;
            binding->m_start_world_pt = world_pt;
            binding->m_pre_screen_pt = screen_pt;
            binding->m_pre_world_pt = world_pt;
            binding->m_cur_screen_pt = screen_pt;
            binding->m_cur_world_pt = world_pt;

            ui_sprite_touch_responser_on_begin(responser, binding_pos);

            if (responser->m_is_start && responser->m_is_grab) {
                ui_sprite_touch_mgr_cancel_other_active(mgr, responser);
                return;
            }
        }
        
        responser = next;
    }
}

static void ui_sprite_touch_process_trace_update_one(
    ui_sprite_touch_responser_t  responser, int8_t binding_pos, UI_SPRITE_2D_PAIR screen_pt)
{
    ui_sprite_touch_responser_binding_t binding;
    UI_SPRITE_2D_PAIR world_pt = ui_sprite_touch_logic_to_world(responser->m_touchable, screen_pt);

    assert(binding_pos >= 0 && binding_pos < responser->m_binding_count);

    binding = &responser->m_bindings[binding_pos];

    binding->m_pre_screen_pt = binding->m_cur_screen_pt;
    binding->m_pre_world_pt = binding->m_cur_world_pt;
    binding->m_cur_screen_pt = screen_pt;
    binding->m_cur_world_pt = world_pt;
        
    if (responser->m_is_capture || ui_sprite_touch_touchable_is_point_in(responser->m_touchable, world_pt)) {
        ui_sprite_touch_responser_on_move(responser, binding_pos);
    }
    else {
        ui_sprite_touch_responser_on_end(responser, binding_pos);
        ui_sprite_touch_responser_unbind_tracer(responser, binding->m_trace);
    }
}

static void ui_sprite_touch_process_trace_update(ui_sprite_touch_mgr_t mgr, ui_sprite_touch_trace_t trace, UI_SPRITE_2D_PAIR screen_pt) {
    ui_sprite_touch_responser_t  responser;
    
    if ((responser = ui_sprite_touch_mgr_find_grab_active_responser(mgr))) {
        int8_t binding_pos = ui_sprite_touch_responser_binding_find(responser, trace);
        if (binding_pos < 0) return;
        ui_sprite_touch_process_trace_update_one(responser, binding_pos, screen_pt);
        return;
    }

    for(responser = TAILQ_FIRST(&trace->m_active_responsers);
        responser != TAILQ_END(&trace->m_active_responsers);
        )
    {
        ui_sprite_touch_responser_t next;
        int8_t binding_pos = ui_sprite_touch_responser_binding_find(responser, trace);
        assert(binding_pos >= 0);

        next = TAILQ_NEXT(responser, m_bindings[binding_pos].m_next);

        ui_sprite_touch_process_trace_update_one(responser, binding_pos, screen_pt);

        if (responser->m_is_start && responser->m_is_grab) {
            ui_sprite_touch_mgr_cancel_other_active(mgr, responser);
            return;
        }
        else {
            responser = next;
        }
    }
}

tl_time_t ui_sprite_touch_mgr_cur_time(ui_sprite_touch_mgr_t mgr) {
    return tl_manage_time(gd_app_tl_mgr(mgr->m_app));
}

static void ui_sprite_touch_process_trace_end(ui_sprite_touch_trace_t trace) {
    while(!TAILQ_EMPTY(&trace->m_active_responsers)) {
        ui_sprite_touch_responser_t responser = TAILQ_FIRST(&trace->m_active_responsers);
        int8_t binding_pos = ui_sprite_touch_responser_binding_find(responser, trace);
        assert(binding_pos >= 0 && binding_pos < responser->m_binding_count);

        ui_sprite_touch_responser_on_end(responser, binding_pos);
        ui_sprite_touch_responser_unbind_tracer(responser, trace);
    }
}

int ui_sprite_touch_mgr_touch_begin(ui_sprite_touch_mgr_t mgr, int32_t touch_id, UI_SPRITE_2D_PAIR screen_pt) {
    ui_sprite_touch_trace_t trace;

    trace = ui_sprite_touch_trace_find(mgr, touch_id);
    if (trace == NULL) {
        trace = ui_sprite_touch_trace_create(mgr, touch_id);
        if (trace == NULL) {
            CPE_ERROR(mgr->m_em, "%s: touch begin: create trace fail!", ui_sprite_touch_mgr_name(mgr));
            return -1;
        }
        ui_sprite_touch_process_trace_begin(mgr, trace, screen_pt);
    }
    else {
        ui_sprite_touch_process_trace_update(mgr, trace, screen_pt);
    }

    return 0;
}

int ui_sprite_touch_mgr_touch_move(ui_sprite_touch_mgr_t mgr, int32_t touch_id, UI_SPRITE_2D_PAIR screen_pt) {
    ui_sprite_touch_trace_t trace;

    trace = ui_sprite_touch_trace_find(mgr, touch_id);
    if (trace == NULL) {
        trace = ui_sprite_touch_trace_create(mgr, touch_id);
        if (trace == NULL) {
            CPE_ERROR(mgr->m_em, "%s: touch begin: create trace fail!", ui_sprite_touch_mgr_name(mgr));
            return -1;
        }
        ui_sprite_touch_process_trace_begin(mgr, trace, screen_pt);
    }
    else {
        ui_sprite_touch_process_trace_update(mgr, trace, screen_pt);
    }

    return 0;
}

int ui_sprite_touch_mgr_touch_end(ui_sprite_touch_mgr_t mgr, int32_t touch_id, UI_SPRITE_2D_PAIR screen_pt) {
    ui_sprite_touch_trace_t trace;

    trace = ui_sprite_touch_trace_find(mgr, touch_id);
    if (trace == NULL) return 0;

    ui_sprite_touch_process_trace_update(mgr, trace, screen_pt);
    ui_sprite_touch_process_trace_end(trace);

    ui_sprite_touch_trace_free(mgr, trace);

    return 0;
}

EXPORT_DIRECTIVE
int ui_sprite_touch_mgr_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    ui_sprite_touch_mgr_t ui_sprite_touch_mgr;
    ui_sprite_repository_t repo;
    ui_sprite_fsm_module_t fsm_module;

    repo = ui_sprite_repository_find_nc(app, cfg_get_string(cfg, "ui-sprite-repository", "ui_sprite_repository"));
    if (repo == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: repository %s not exist",
            gd_app_module_name(module),
            cfg_get_string(cfg, "ui-sprite-repository", "ui_sprite_repository"));
        return -1;
    }

    fsm_module = ui_sprite_fsm_module_find_nc(app, cfg_get_string(cfg, "ui-sprite-fsm-repository", NULL));
    if (fsm_module == NULL) {
        CPE_ERROR(
            gd_app_em(app), "%s: create: fsm-repository %s not exist",
            gd_app_module_name(module),
            cfg_get_string(cfg, "ui-sprite-fsm-repository", "default"));
        return -1;
    }

    ui_sprite_touch_mgr =
        ui_sprite_touch_mgr_create(
            app, repo, fsm_module,
            gd_app_alloc(app),
            gd_app_module_name(module),
            gd_app_em(app));
    if (ui_sprite_touch_mgr == NULL) return -1;

    ui_sprite_touch_mgr->m_debug = cfg_get_int32(cfg, "debug", 0);

    if (ui_sprite_touch_mgr->m_debug) {
        CPE_INFO(
            gd_app_em(app), "%s: create: done",
            ui_sprite_touch_mgr_name(ui_sprite_touch_mgr));
    }

    return 0;
}

EXPORT_DIRECTIVE
void ui_sprite_touch_mgr_app_fini(gd_app_context_t app, gd_app_module_t module) {
    ui_sprite_touch_mgr_t ui_sprite_touch_mgr;

    ui_sprite_touch_mgr = ui_sprite_touch_mgr_find_nc(app, gd_app_module_name(module));
    if (ui_sprite_touch_mgr) {
        ui_sprite_touch_mgr_free(ui_sprite_touch_mgr);
    }
}

