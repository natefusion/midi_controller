#ifndef HALLEFFECT_H
#define HALLEFFECT_H

#include "misc.h"

/*
  0: q=2.595, s=1.700
  1: q=2.525, s=1.765
  2: q=2.570, s=1.725
  3: q=2.525, s=1.760
  4: q=2.615, s=1.685

  avg: q=2.566, s=1.727
 */

#define WINDOW_SIZE 5

typedef struct Moving_Average {
    u16 sum;
    u16 average;
    u16 readings[WINDOW_SIZE];
    u16 index;
} Moving_Average;

typedef struct Hall_Effect {
    u8 port;

    // these four are measured in adc bits
    
    // these values were collected by measuring the sensor with no magnetic field
    // and with the magnet of choice (not telling) touching the sensor
    u16 operational_max_adc;
    u16 operational_min_adc;

    // these values should be measured when calibrating the keyboard
    // the min should be greater than the operational min
    // the max should be less than the operational max
    u16 max_adc;
    u16 min_adc;

    // measured in mm
    // these are just the numbers from the magic function in halleffect.c at the max_adc and min_adc
    float max_distance;
    float min_distance;

    Moving_Average ma;
} Hall_Effect;

float halleffect_distance_curve(u8 sensor, float index);
float halleffect_get_value(Hall_Effect *sensor, u16 rawADC);

#endif
