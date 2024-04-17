#ifndef HALLEFFECT_H
#define HALLEFFECT_H

#include "misc.h"

/*
  0: q=2.595, s=1.700
  1: q=2.525, s=1.765
  2: q=2.570, s=1.775
  3: q=2.525, s=1.760
  4: q=2.615, s=1.685
 */

typedef struct Hall_Effect {
    u8 port;

    // measured in adc bits
    u16 max_adc;
    u16 min_adc;

    // measured in mm
    float min_distance;
    float max_distance;
} Hall_Effect;

float halleffect_get_value(Hall_Effect *sensor, u16 rawADC);

#endif
