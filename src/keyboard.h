#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "misc.h"
#include <stdbool.h>

typedef struct Key_Hammer {
    float key_pos;
    float key_pos_prev;
    float key_velocity;
    float key_strike_distance;
    
    float hammer_pos;
    float hammer_velocity;
    float hammer_travel;
    
    float gravity;
    bool hammer_is_striking;
    bool key_is_striking;
    bool note_on_sent;
    bool note_off_sent;
} Key_Hammer;

Key_Hammer keyhammer_make(float travel);
void keyhammer_update(Key_Hammer* h, float pos, float dt);

#endif
