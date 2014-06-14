#include <assert.h>
#include "cpe/utils/math_ex.h"
#include "ui_sprite_anim_camera_op_i.h"
#include "ui_sprite_anim_camera_restrict_i.h"

void ui_sprite_anim_camera_op_update_move_by_speed(ui_sprite_anim_camera_op_t camera_op, float delta) {
	ui_sprite_anim_camera_t camera = camera_op->m_camera;
	UI_SPRITE_ANIM_CAMERA_OP_MOVE_BY_SPEED * op = &camera_op->m_op.data.move_by_speed;
	UI_SPRITE_2D_PAIR move_to_pos = camera->m_camera_pos;

	op->left_time += delta;

	while(op->left_time > op->step) {
		if (fabs(op->speed.x) >= op->reduce_per_step.x) {
			move_to_pos.x +=  op->speed.x * op->step;
			op->speed.x -= op->speed.x > 0 ? op->reduce_per_step.x : - op->reduce_per_step.x;
		}
		else {
			op->speed.x = 0;
		}

		if (fabs(op->speed.y) >= op->reduce_per_step.y) {
			move_to_pos.y +=  op->speed.y * op->step;
			op->speed.y -= op->speed.y > 0 ? op->reduce_per_step.y : - op->reduce_per_step.y; 
		}
		else {
			op->speed.y = 0;
		}

		op->left_time -= op->step;

		if(op->speed.x ==0 && op->speed.y == 0)
			break;
	}

	//CPE_ERROR(camera->m_module->m_em, "xxx update pos (%f,%f) ==> (%f,%f)\n", camera->m_camera_pos.x, camera->m_camera_pos.y, move_to_pos.x, move_to_pos.y);
    ui_sprite_anim_camera_set_pos_and_scale(camera, move_to_pos, camera->m_camera_scale);

    if (fabs(op->speed.x) < 0.001f && fabs(op->speed.y) < 0.001f) {
        //printf("xxx is done\n");
        camera_op->m_is_done = 1;
    }
}
