#include <assert.h>
#include "ui/sprite/ui_sprite_world_res.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui_sprite_ctrl_track_mgr_i.h"

static void ui_sprite_ctrl_track_mgr_clear(ui_sprite_world_res_t world_res, void * ctx);

ui_sprite_ctrl_track_mgr_t
ui_sprite_ctrl_track_mgr_create(ui_sprite_ctrl_module_t module, ui_sprite_world_t world) {
    ui_sprite_ctrl_track_mgr_t track_mgr;
    ui_sprite_world_res_t world_res = ui_sprite_world_res_create(world, UI_SPRITE_CTRL_TRACK_MGR_TYPE_NAME, sizeof(struct ui_sprite_ctrl_track_mgr));

    track_mgr = ui_sprite_world_res_data(world_res);

    track_mgr->m_module = module;
    TAILQ_INIT(&track_mgr->m_metas);
    TAILQ_INIT(&track_mgr->m_tracks);

    ui_sprite_world_res_set_free_fun(world_res, ui_sprite_ctrl_track_mgr_clear, module);

    return track_mgr;
}

static void ui_sprite_ctrl_track_mgr_clear(ui_sprite_world_res_t world_res, void * ctx) {
    ui_sprite_ctrl_track_mgr_t track_mgr;

    track_mgr = ui_sprite_world_res_data(world_res);

	while(!TAILQ_EMPTY(&track_mgr->m_tracks)) {
		ui_sprite_ctrl_track_free(TAILQ_FIRST(&track_mgr->m_tracks));
	}

    while(!TAILQ_EMPTY(&track_mgr->m_metas)) {
        ui_sprite_ctrl_track_meta_free(TAILQ_FIRST(&track_mgr->m_metas));
    }
}

void ui_sprite_ctrl_track_mgr_free(ui_sprite_ctrl_track_mgr_t track_mgr) {
    ui_sprite_world_res_t world_res = ui_sprite_world_res_from_data(track_mgr);
    ui_sprite_world_res_free(world_res);
}

ui_sprite_ctrl_track_mgr_t ui_sprite_ctrl_track_mgr_find(ui_sprite_world_t world) {
    ui_sprite_world_res_t world_res = ui_sprite_world_res_find(world, UI_SPRITE_CTRL_TRACK_MGR_TYPE_NAME);
    return world_res ? ui_sprite_world_res_data(world_res) : NULL;
}

const char * UI_SPRITE_CTRL_TRACK_MGR_TYPE_NAME = "TrackMgr";