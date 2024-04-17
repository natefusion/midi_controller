#include "halleffect.h"
#include "adc.h"
/* #include <avr/pgmspace.h> */
#include <math.h>

float halleffect_get_value(u8 sensor, u16 rawADC) {
    static u16 min_adc_value = 536;

    if (rawADC < min_adc_value) {
        return 0.0f;
    }

    u16 index = rawADC - min_adc_value;
    if (index >= 316) {
        return 0.0f;
    }

    // magic numbers. oooooooooooh. aaaaaaaaaaaaah
    switch (sensor) {
    case 0:
        return 42.389263f * powf(index, -0.43429285f);
    case 1:
        return 42.208714f *  powf(index, -0.4303872f);
    case 2:
        return 42.918343f * powf(index, -0.4331107f);
    case 3:
        return 42.17821f * powf(index, -0.43052763f);
    case 4:
        return 42.487366f * powf(index, -0.4354517f);
    default:
        return 0.0f;
    }
}
