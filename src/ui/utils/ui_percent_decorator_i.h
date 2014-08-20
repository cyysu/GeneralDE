#ifndef UI_UTILS_PERCENT_DECORATE_I_H
#define UI_UTILS_PERCENT_DECORATE_I_H
#include "ui/utils/ui_percent_decorator.h"

#ifdef __cplusplus
extern "C" {
#endif

union ui_percent_decorator_data {
    struct {
        float m_rate;
    } m_ease;
    struct {
        float m_period;
    } m_elastic;
};

#ifdef __cplusplus
}
#endif

#endif
