#include <assert.h>
#include "cpe/utils/math_ex.h"

float cpe_math_distance(float x1, float y1, float x2, float y2) {
	return (float)sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
}

float cpe_math_angle(float x1, float y1, float x2, float y2) {
    float diff_x = x2 - x1;
    float diff_y = y2 - y1;
    float angle;

    if (fabs(diff_x) < 0.01f) {
        if (fabs(diff_y) < 0.01) {
            return -90.f;
        }
        else if (fabs(diff_y) < 0.01) {
            assert(0);
            return 0.0f;
        }
        else {
            return 90.0f;
        }
    }

    angle = atan2f(diff_y, diff_x) * 180.0f / M_PI;

    assert(angle <= 180.f && angle >= -180.f);

    return angle;
}

float cpe_math_angle_regular(float angle) {
    while(angle > 180.0f) angle -= 360.0;
    while(angle < -180.0f) angle += 360.0;
    return angle;
}

float cpe_math_angle_add(float angle_1, float angle_2) {
	float r = 0.0f;
    assert(angle_1 <= 180.f && angle_1 >= -180.f);
    assert(angle_2 <= 180.f && angle_2 >= -180.f);

    r = angle_1 + angle_2;

    return r > 180.0f ? (r - 360.f)
        : r < -180.0f ? (r + 360.f)
        : r;
}

float cpe_math_angle_diff(float angle_1, float angle_2) {
	float r = 0.0f;
    assert(angle_1 <= 180.f && angle_1 >= -180.f);
    assert(angle_2 <= 180.f && angle_2 >= -180.f);

    r = angle_2 - angle_1;

    return r > 180.0f ? (r - 360.f)
        : r < -180.0f ? (r + 360.f)
        : r;
}

float cpe_math_radians(float x1, float y1, float x2, float y2) {
    float diff_x = x2 - x1;
    float diff_y = y2 - y1;
    float r;

    if (fabs(diff_x) < 0.01f) {
        if (fabs(diff_y) < 0.01) {
            return (float)- M_PI_2;
        }
        else if (fabs(diff_y) < 0.01) {
            assert(0);
            return 0.0f;
        }
        else {
            return (float)M_PI_2;
        }
    }

    r = atan2f(diff_y, diff_x);

    return r > M_PI ? (r - 2 * M_PI)
    : r < - M_PI ? (r + 2 * M_PI)
    : r;
}

float cpe_math_radians_regular(float radians) {
    while(radians > M_PI) radians -= (float)(2 * M_PI);
    while(radians < - M_PI) radians += (float)(2 * M_PI);
    return radians;
}

float cpe_math_radians_add(float radians_1, float radians_2) {
	float r = 0.0f;
    assert(radians_1 <= M_PI && radians_1 >= -M_PI);
    assert(radians_2 <= M_PI && radians_2 >= -M_PI);

    r = radians_1 + radians_2;

    return r > M_PI ? (r - 2 * M_PI)
        : r < - M_PI ? (r + 2 * M_PI)
        : r;
}

float cpe_math_radians_diff(float radians_1, float radians_2) {
	float r = 0.0f;
    assert(radians_1 <= M_PI && radians_1 >= -M_PI);
    assert(radians_2 <= M_PI && radians_2 >= -M_PI);

    r = radians_1 - radians_2;

    return r > M_PI ? (float)(r - 2 * M_PI)
        : r < - M_PI ? (float)(r + 2 * M_PI)
        : r;
}
