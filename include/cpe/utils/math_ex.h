#ifndef CPE_UTILS_MATH_EX_H
#define CPE_UTILS_MATH_EX_H
#include "cpe/pal/pal_math.h"

#ifdef __cplusplus
extern "C" {
#endif

float cpe_math_distance(float x1, float y1, float x2, float y2);

/*角度 */
float cpe_math_angle(float x1, float y1, float x2, float y2);
float cpe_math_angle_add(float angle_1, float angle_2);
float cpe_math_angle_diff(float angle_1, float angle_2);
float cpe_math_angle_regular(float angle);

/*弧度 */
float cpe_math_radians(float x1, float y1, float x2, float y2);
float cpe_math_radians_add(float angle_1, float angle_2);
float cpe_math_radians_diff(float angle_1, float angle_2);
float cpe_math_radians_regular(float radians);

/*角度弧度转换 */
#define cpe_math_angle_to_radians(__angle) ( (__angle) * M_PI / 180.f )
#define cpe_math_raidans_to_angle(__radians) ( (__radians) * 180.0f * M_1_PI )

#define cpe_cos_angle(__angle) cos(cpe_math_angle_to_radians(__angle))
#define cpe_cos_radians(__radians) cos(__radians)

#define cpe_sin_angle(__angle) sin(cpe_math_angle_to_radians(__angle))
#define cpe_sin_radians(__radians) sin(__radians)

#ifdef __cplusplus
}
#endif

#endif
