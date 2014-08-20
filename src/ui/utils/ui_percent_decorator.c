#include "cpe/pal/pal_types.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/math_ex.h"
#include "cpe/utils/string_utils.h"
#include "ui_percent_decorator_i.h"

/* ease */
static float ui_percent_decorator_ease_in(ui_percent_decorator_t decorator, float input) {
    union ui_percent_decorator_data * d = (union ui_percent_decorator_data *)(decorator->m_data);
    return powf(input, d->m_ease.m_rate);
}

static float ui_percent_decorator_ease_out(ui_percent_decorator_t decorator, float input) {
    union ui_percent_decorator_data * d = (union ui_percent_decorator_data *)(decorator->m_data);
    return powf(input, 1 / d->m_ease.m_rate);
}

static float ui_percent_decorator_ease_inout(ui_percent_decorator_t decorator, float input) {
    union ui_percent_decorator_data * d = (union ui_percent_decorator_data *)(decorator->m_data);

    input *= 2;
    if (input < 1.0f) {
        return 0.5f * powf(input, d->m_ease.m_rate);
    }
    else {
        return 1.0f - 0.5f * powf(2.0f - input, d->m_ease.m_rate);
    }
}

static int ui_percent_decorator_setup_ease(ui_percent_decorator_t decorator, ui_percent_decorator_fun_t fun, char * args, error_monitor_t em) {
    union ui_percent_decorator_data * d = (union ui_percent_decorator_data *)(decorator->m_data);
    char * value;

    value = cpe_str_read_and_remove_arg(args, "rate", ',', '=');
    if (value == NULL) {
        CPE_ERROR(em, "ui_percent_decorator: ease: arg rate not configured!");
        return -1;
    }

    decorator->m_fun = fun;
    d->m_ease.m_rate = strtof(value, NULL);

    return 0;
}

/* exponential */
static float ui_percent_decorator_ease_exponential_in(ui_percent_decorator_t decorator, float input) {
    return input == 0 ? 0 : powf(2, 10 * (input / 1 - 1)) - 1 * 0.001f;
}

static float ui_percent_decorator_ease_exponential_out(ui_percent_decorator_t decorator, float input) {
    return input >= 1.0f ? 1.0f : (- powf(2, -10 * input / 1) + 1);
}

static float ui_percent_decorator_ease_exponential_inout(ui_percent_decorator_t decorator, float input) {
    input /= 0.5f;
    if (input < 1) {
        input = 0.5f * powf(2, 10 * (input - 1));
    }
    else {
        input = 0.5f * (-powf(2, -10 * (input - 1)) + 2);
    }

    return input;
}

/* sine */
static float ui_percent_decorator_ease_sine_in(ui_percent_decorator_t decorator, float input) {
    return -1 * cosf(input * (float)M_PI_2) + 1;
}

static float ui_percent_decorator_ease_sine_out(ui_percent_decorator_t decorator, float input) {
    return sinf(input * (float)M_PI_2);
}

static float ui_percent_decorator_ease_sine_inout(ui_percent_decorator_t decorator, float input) {
    return -0.5f * (cosf((float)M_PI * input) - 1);
}

/* elastic */
static float ui_percent_decorator_ease_elastic_in(ui_percent_decorator_t decorator, float input) {
    union ui_percent_decorator_data * d = (union ui_percent_decorator_data *)(decorator->m_data);
    float new_value = 0;

    if (input == 0 || input == 1) {
        new_value = input;
    }
    else {
        float s = d->m_elastic.m_period / 4;
        input = input - 1;
        new_value = -powf(2, 10 * input) * sinf((input - s) * M_PI_X_2 / d->m_elastic.m_period);
    }

    return new_value;
}

static float ui_percent_decorator_ease_elastic_out(ui_percent_decorator_t decorator, float input) {
    union ui_percent_decorator_data * d = (union ui_percent_decorator_data *)(decorator->m_data);
    float new_value = 0;

    if (input == 0 || input == 1) {
        new_value = input;
    }
    else {
        float s = d->m_elastic.m_period / 4;
        new_value = powf(2, -10 * input) * sinf((input - s) * M_PI_X_2 / d->m_elastic.m_period) + 1;
    }

    return new_value;
}

static float ui_percent_decorator_ease_elastic_inout(ui_percent_decorator_t decorator, float input) {
    union ui_percent_decorator_data * d = (union ui_percent_decorator_data *)(decorator->m_data);
    float new_value = 0;

    if (input == 0 || input == 1) {
        new_value = input;
    }
    else {
        float s;

        input = input * 2;
        if (!d->m_elastic.m_period) {
            d->m_elastic.m_period = 0.3f * 1.5f;
        }

        s = d->m_elastic.m_period / 4;
        input = input - 1;
        if (input < 0) {
            new_value = -0.5f * powf(2, 10 * input) * sinf((input -s) * M_PI_X_2 / d->m_elastic.m_period);
        }
        else {
            new_value = powf(2, -10 * input) * sinf((input - s) * M_PI_X_2 / d->m_elastic.m_period) * 0.5f + 1;
        }
    }

    return new_value;
}

static int ui_percent_decorator_setup_elastic(ui_percent_decorator_t decorator, ui_percent_decorator_fun_t fun, char * args, error_monitor_t em) {
    union ui_percent_decorator_data * d = (union ui_percent_decorator_data *)(decorator->m_data);
    char * value;

    value = cpe_str_read_and_remove_arg(args, "period", ',', '=');
    if (value == NULL) {
        CPE_ERROR(em, "ui_percent_decorator: ease: arg rate not configured!");
        return -1;
    }

    decorator->m_fun = fun;
    d->m_ease.m_rate = strtof(value, NULL);

    return 0;
}

/* bounce */
static float ui_percent_decorator_bounse_do(float input) {
    if (input < 1 / 2.75) {
        return 7.5625f * input * input;
    } 
    else if (input < 2 / 2.75) {
        input -= 1.5f / 2.75f;
        return 7.5625f * input * input + 0.75f;
    }
    else if(input < 2.5 / 2.75) {
        input -= 2.25f / 2.75f;
        return 7.5625f * input * input + 0.9375f;
    }

    input -= 2.625f / 2.75f;

    return 7.5625f * input * input + 0.984375f;
}

static float ui_percent_decorator_ease_bounse_in(ui_percent_decorator_t decorator, float input) {
    return ui_percent_decorator_bounse_do(1.0f - input);
}

static float ui_percent_decorator_ease_bounse_out(ui_percent_decorator_t decorator, float input) {
    return ui_percent_decorator_bounse_do(input);
}

static float ui_percent_decorator_ease_bounse_inout(ui_percent_decorator_t decorator, float input) {
    float new_value = 0;

    if (input < 0.5f) {
        input = input * 2;
        new_value = (1 - ui_percent_decorator_bounse_do(1 - input)) * 0.5f;
    }
    else {
        new_value = ui_percent_decorator_bounse_do(input * 2 - 1) * 0.5f + 0.5f;
    }

    return new_value;
}

/* back */
static float ui_percent_decorator_ease_back_in(ui_percent_decorator_t decorator, float input) {
    float overshoot = 1.70158f;
    return input * input * ((overshoot + 1) * input - overshoot);
}

static float ui_percent_decorator_ease_back_out(ui_percent_decorator_t decorator, float input) {
    float overshoot = 1.70158f;
    input = input - 1;
    return input * input * ((overshoot + 1) * input + overshoot) + 1;
}

static float ui_percent_decorator_ease_back_inout(ui_percent_decorator_t decorator, float input) {
    float overshoot = 1.70158f * 1.525f;

    input = input * 2;
    if (input < 1) {
        return (input * input * ((overshoot + 1) * input - overshoot)) / 2;
    }
    else {
        input = input - 2;
        return (input * input * ((overshoot + 1) * input + overshoot)) / 2 + 1;
    }
}

/* public */
int ui_percent_decorator_setup(ui_percent_decorator_t decorator, const char * input_def, error_monitor_t em) {
    char buf[64];
    char * def;
    char * sep;
    char * args;
    char * name;

    strncpy(buf, input_def, sizeof(buf));

    def = cpe_str_trim_head(buf);

    if ((sep = strchr(def, ':'))) {
        *sep = 0;
        *cpe_str_trim_tail(sep, def) = 0;

        name = def;
        args = cpe_str_trim_head(sep + 1); 
    }
    else {
        args = cpe_str_trim_tail(def + strlen(def), def);
        *args = 0;
        name = def;
    }

    decorator->m_fun = NULL;

    if (strcmp(name, "ease-in") == 0) {
        return ui_percent_decorator_setup_ease(decorator, ui_percent_decorator_ease_in, args, em);
    }
    else if (strcmp(name, "ease-inout") == 0) {
        return ui_percent_decorator_setup_ease(decorator, ui_percent_decorator_ease_inout, args, em);
    }
    else if (strcmp(name, "ease-out") == 0) {
        return ui_percent_decorator_setup_ease(decorator, ui_percent_decorator_ease_out, args, em);
    }
    else if (strcmp(name, "ease-exponential-in") == 0) {
        decorator->m_fun = ui_percent_decorator_ease_exponential_in;
        return 0;
    }
    else if (strcmp(name, "ease-exponential-out") == 0) {
        decorator->m_fun = ui_percent_decorator_ease_exponential_out;
        return 0;
    }
    else if (strcmp(name, "ease-exponential-inout") == 0) {
        decorator->m_fun = ui_percent_decorator_ease_exponential_inout;
        return 0;
    }
    else if (strcmp(name, "ease-sine-in") == 0) {
        decorator->m_fun = ui_percent_decorator_ease_sine_in;
        return 0;
    }
    else if (strcmp(name, "ease-sine-out") == 0) {
        decorator->m_fun = ui_percent_decorator_ease_sine_out;
        return 0;
    }
    else if (strcmp(name, "ease-sine-inout") == 0) {
        decorator->m_fun = ui_percent_decorator_ease_sine_inout;
        return 0;
    }
    else if (strcmp(name, "ease-elastic-in") == 0) {
        return ui_percent_decorator_setup_elastic(decorator, ui_percent_decorator_ease_elastic_in, args, em);
    }
    else if (strcmp(name, "ease-elastic-out") == 0) {
        return ui_percent_decorator_setup_elastic(decorator, ui_percent_decorator_ease_elastic_out, args, em);
    }
    else if (strcmp(name, "ease-elastic-inout") == 0) {
        return ui_percent_decorator_setup_elastic(decorator, ui_percent_decorator_ease_elastic_inout, args, em);
    }
    else if (strcmp(name, "ease-bounse-in") == 0) {
        decorator->m_fun = ui_percent_decorator_ease_bounse_in;
        return 0;
    }
    else if (strcmp(name, "ease-bounse-out") == 0) {
        decorator->m_fun = ui_percent_decorator_ease_bounse_out;
        return 0;
    }
    else if (strcmp(name, "ease-bounse-inout") == 0) {
        decorator->m_fun = ui_percent_decorator_ease_bounse_inout;
        return 0;
    }
    else if (strcmp(name, "ease-back-in") == 0) {
        decorator->m_fun = ui_percent_decorator_ease_back_in;
        return 0;
    }
    else if (strcmp(name, "ease-back-out") == 0) {
        decorator->m_fun = ui_percent_decorator_ease_back_out;
        return 0;
    }
    else if (strcmp(name, "ease-back-inout") == 0) {
        decorator->m_fun = ui_percent_decorator_ease_back_inout;
        return 0;
    }
    else {
        CPE_ERROR(em, "ui_percent_decorator: support percent decorator %s", name);
        return -1;
    }
}

float ui_percent_decorator_decorate(ui_percent_decorator_t decorator, float input) {
    return decorator->m_fun
        ? decorator->m_fun(decorator, input)
        : input;
}
