#ifndef CPE_UTILS_MATH_EX_H
#define CPE_UTILS_MATH_EX_H
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

float cpe_math_distance(float x1, float y1, float x2, float y2);
float cpe_math_angle(float x1, float y1, float x2, float y2);

#ifdef __cplusplus
}
#endif

#endif
