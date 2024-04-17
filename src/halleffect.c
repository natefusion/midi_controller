#include "halleffect.h"
#include "adc.h"
#include <math.h>

float halleffect_get_value(Hall_Effect *sensor, u16 raw_adc) {
    u16 index = raw_adc - sensor->min_adc;

    // magic numbers. oooooooooooh. aaaaaaaaaaaaah
    switch (sensor->port) {
    case 0:
        return 42.389263f * powf(index, -0.43429285f);
    case 1:
        return 42.208714f *  powf(index, -0.4303872f);
    case 2:
        return 42.918343f * powf(index, -0.4331107f);
    case 3:
        return 42.17821f * powf(index, -0.43052763f);
    case 4:
        if (raw_adc <= sensor->min_adc) {
            return 31.417f;
        }

        if (raw_adc >= sensor->max_adc) {
            return 3.885469467f;
        }
        
        return 42.487366f * powf(index, -0.4354517f);
    default:
        return 0.0f;
    }
}
