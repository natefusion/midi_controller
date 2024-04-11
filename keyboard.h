#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "misc.h"

typedef struct Key {
    u16 pos;
    u16 pos_prev;
    u16 pos_min;
    u16 speed; 
    u16 travel;
    u16 len;
} Key;

typedef struct Hammer {
    u16 pos;
    u16 speed;
    u16 max_speed;
    u16 travel;
    u16 gravity;
    Key key;
} Hammer;

bool key_has_struck(Key k);
void key_update(Key k, u16 new_pos);

Hammer hammer_make(void);
bool hammer_has_struck(Hammer kh);
bool hammer_update(Hammer h, u16 pos, u16 dt);

#endif
