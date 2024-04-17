#include "keyboard.h"

Hammer hammer_make(float min_sensor_value, float max_sensor_value) {
    Hammer h;
    h.key = (Key) {
        .pos = min_sensor_value,
        .pos_prev = min_sensor_value,
        .pos_min = min_sensor_value,
        .pos_max = max_sensor_value,
        .speed = 0.0f,
    };

    h.note_sent = 0;
    h.pos = 0.0f;
    h.speed = 0.0f;
    h.travel = 8;// (max_sensor_value - min_sensor_value) * 5;
    
    // 38.307 <- (femtometers / cycle^2),  (millimeters / second^2) ->  9806.65f / 5;
    h.gravity = 9806.65f / 5.0f;

    return h;
}

u8 key_update(Key* k, float pos, float dt) {
    k->pos_prev = k->pos;
    k->pos = pos;
    k->speed = (k->pos - k->pos_prev) / dt;

    if (pos < 4.5f) {
        return 1;
    } else {
        return 0;
    }
}

u8 hammer_update(Hammer* h, float pos, float dt) {
    h->key.pos_prev = h->key.pos;
    h->key.pos = pos;
    h->key.speed = (h->key.pos - h->key.pos_prev) / dt;

    float original_speed = h->speed;
    h->speed -= h->gravity * dt;
    h->pos += (original_speed + h->speed) * dt / 2;

    if (h->pos < h->key.pos) {
        h->pos = h->key.pos;
        if (h->speed < h->key.speed) {
            h->speed = h->key.speed;
        }
    }

    u8 note_on = h->pos > h->travel;
    if (note_on) {
        h->speed = -h->speed;
        h->pos = h->travel;
    };

    return note_on;
}
