#include "gd/app/app_log.h"
#include "cpe/utils/math_ex.h"
#include "ui_sprite_ctrl_turntable_i.h"

void ui_sprite_ctrl_turntable_track_circle(
    ui_sprite_ctrl_turntable_t turntable, float scale, 
    UI_SPRITE_2D_PAIR * r, UI_SPRITE_2D_PAIR const * base, float angle)
{
	float radians = cpe_math_angle_to_radians(angle);
    float radius = turntable->m_def.track_def.circle.radius * scale;
    r->x = base->x + radius * cos(radians);
    r->y = base->y + radius * sin(radians);
}

void ui_sprite_ctrl_turntable_track_ellipse(
    ui_sprite_ctrl_turntable_t turntable, float scale, 
    UI_SPRITE_2D_PAIR * r, UI_SPRITE_2D_PAIR const * base, float angle)
{
    float ellipse_radians = cpe_math_angle_to_radians(turntable->m_def.track_def.ellipse.angle);
	float input_radians = cpe_math_angle_to_radians(angle);
	float radians_in_ellipse = input_radians - ellipse_radians;

	float radius_x = turntable->m_def.track_def.ellipse.radius_x * scale;
	float radius_y = turntable->m_def.track_def.ellipse.radius_y * scale;
	float cos_radians_in_ellipse = cos(radians_in_ellipse);
	float sin_radians_in_ellipse = sin(radians_in_ellipse);

	float polar = 
        radius_x * radius_y
        / ( sqrt(radius_x * radius_x * sin_radians_in_ellipse * sin_radians_in_ellipse 
                 + radius_y * radius_y * cos_radians_in_ellipse * cos_radians_in_ellipse
                )
            );

    r->x = base->x + polar * cos(input_radians);
    r->y = base->y + polar * sin(input_radians);
}