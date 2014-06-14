#define _USE_MATH_DEFINES
#include "cpe/utils/math_ex.h"

float cpe_math_distance(float x1, float y1, float x2, float y2) {
	return sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
}

float cpe_math_angle(float x1, float y1, float x2, float y2) {
    return atan2f(y2 - y1, x2 - x1) * 180.0f / M_PI;
}
