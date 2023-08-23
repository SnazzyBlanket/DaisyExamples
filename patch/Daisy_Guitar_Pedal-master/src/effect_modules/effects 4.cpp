#include "effects.h"

effects::effects()
{
    // All parameters are not enabled by default
    for (uint8_t i = 0; i < MAX_PARAM_NUM; i++)
    {
        param[i].enable = false;
        param[i].value = 0;
    }
    // effects are bypassed by default
    enable = false;
}

void effects::init_param(unsigned char id, const char* name, bool enable, unsigned char value, param_mode mode, char prec, const char* unit)
{
    strcpy(param[id].name, name);
    param[id].enable = enable;
    param[id].value = value;
    param[id].mode = mode;
    param[id].prec = prec;
    strcpy(param[id].unit, unit);
    set_param(id, value);
}

// A global variable to store sample rate
float sample_rate;