#include <math.h>
#include <stdbool.h>

#include "halleffect.h"
#include "adc.h"

Hall_Effect halleffect_make(u8 port, u16 op_min_adc, u16 op_max_adc, u16 min_adc, u16 max_adc) {
    return (Hall_Effect) {
        .port = port,
        .operational_min_adc = op_min_adc,
        .operational_max_adc = op_max_adc,
        .min_adc = min_adc,
        .max_adc = max_adc,
        .max_distance = halleffect_distance_curve(port, 1.0f),
        .min_distance = halleffect_distance_curve(port, max_adc - min_adc + 1),

        // haha. initialization matters.
        // I know this is automatically initialized to zero when a struct is created like this
        // but now it is certainly initialized
        // without a doubt
        // definitely
        .ma = {0}, 
    };
}

u16 movingaverage_process(Moving_Average *ma, u16 raw_adc) {
    ma->sum -= ma->readings[ma->index];
    ma->readings[ma->index] = raw_adc;
    ma->sum += raw_adc;
    ma->index = (ma->index+1) % WINDOW_SIZE;
    ma->average = ma->sum / WINDOW_SIZE;

    return ma->average;
}

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
     u16 averaged_adc = movingaverage_process(&sensor->ma, raw_adc);
    
    // less than this means sensor is not calibrated or sensor jitter
    // it should always be greater than operational_min_adc
    // so we don't need to worry about leaving the function range
    if (averaged_adc < sensor->min_adc)
        averaged_adc = sensor->min_adc;

    // higher than this and the functions stop working...
    if (averaged_adc > sensor->operational_max_adc)
        averaged_adc = sensor->operational_max_adc;
    
    float index = (float)(averaged_adc - sensor->min_adc + 1);
    float offset = 3.0f;

    if (averaged_adc >= sensor->max_adc) {
        index = sensor->max_adc - sensor->min_adc + 1;
    }

    // We want the number to go up, not down, so subtract distance from sensor from max distance
    return sensor->max_distance - halleffect_distance_curve(sensor->port, index + offset);
}
