#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "misc.h"

typedef struct Key {
    float pos;
    float pos_prev;
    float pos_min;
    float pos_max;
    float speed; 
} Key;

typedef struct Hammer {
    float pos;
    float speed;
    float travel;
    float gravity;
    Key key;
} Hammer;

Hammer hammer_make(float min_sensor_value, float max_sensor_value);
u8 hammer_update(Hammer* h, float pos, float dt);

#endif
