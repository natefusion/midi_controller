#include "keyboard.h"

Key key_make(void) {
    return (Key) {
        .pos = 0,
        .pos_prev = 0,
        .pos_min = 0,
        .speed = 0,
        .travel = 0,
        .len = 0,
    };
}

Hammer hammer_make(void) {
    Hammer h;
    h.key = key_make();
    h.pos = 0;
    h.speed = 0;
    h.max_speed = 0;
    h.travel = 0;
    h.gravity = (980 / h.travel) * h.key.travel;

    return h;
}

void key_update(Key k, u16 new_pos) {
    k.pos_prev = k.pos;
    k.pos = new_pos;
}

void hammer_update_speed(Hammer h, u16 dt) {
    h.key.speed = 3*(h.key.pos - h.key.pos_prev) / dt;

    u16 original_speed = h.speed;
    h.speed -= h.gravity * dt;
    h.pos += (original_speed + h.speed) * dt / 2;
}

void hammer_check(Hammer h) {
    if (h.pos < h.key.pos) {
        h.pos = h.key.pos;
        if (h.speed < h.key.speed) {
            h.speed = h.key.speed;
        }
    }
}

bool hammer_has_struck(Hammer h) {
    return h.pos > h.key.travel * 1.1;
}

bool key_has_struck(Key k) {
    // TODO:
    return false;
}

bool hammer_update(Hammer h, u16 pos, u16 delta_time) {
    key_update(h.key, h.key.len * pos);
    hammer_update_speed(h, delta_time);
    hammer_check(h);
    return hammer_has_struck(h);
}
