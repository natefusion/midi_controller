#include "halleffect.h"
#include "adc.h"
#include <math.h>

float halleffect_distance_curve(u8 port, float index) {
    // magic numbers. oooooooooooh. aaaaaaaaaaaaah
    switch (port) {
    case 0:
        return 45.958904f * powf(index, -0.44967207f);
    case 1:
        return 45.426605f * powf(index, -0.4443363f);
    case 2:
        return 45.75797f * powf(index, -0.4476087f);
    case 3:
        return 45.3737f * powf(index, -0.44436312f);
    case 4:
        return 46.21221f * powf(index, -0.4514614f);
    default:
        // this should not execute. just here to make the compiler happy
        return 0.0f;
    }
}

float halleffect_get_value(Hall_Effect *sensor, u16 raw_adc) {
    // less than this means sensor is not calibrated or sensor jitter
    // it should always be greater than operational_min_adc
    // so we don't need to worry about leaving the function range
    if (raw_adc < sensor->min_adc)
        raw_adc = sensor->min_adc;

    // higher than this and the functions stop working...
    if (raw_adc > sensor->operational_max_adc)
        raw_adc = sensor->operational_max_adc;
    
    u16 index = raw_adc - sensor->min_adc + 1;
    u16 offset = 3; // the first values are not good, tweak this as necessary
    
    if (raw_adc >= sensor->max_adc) {
        index = sensor->max_adc - sensor->min_adc + 1;
    }
    

    // We want the number to go up, not down, so subtract distance from sensor from max distance
    return sensor->max_distance - halleffect_distance_curve(sensor->port, (float)(index + offset));
}
