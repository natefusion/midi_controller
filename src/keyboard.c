#include "keyboard.h"

Key_Hammer keyhammer_make(float travel) {
    // velocity is measured in mm/s
    // position is measured in mm
    return (Key_Hammer) {
        .key_pos = 0.0f,
        .key_pos_prev = 0.0f,
        .key_velocity = 0.0f,
        .hammer_pos = 0.0f,
        .hammer_velocity = 0.0f,
        .hammer_travel = travel,
        .key_strike_distance = travel - 2.0f,
        .gravity = 9806.65f,
        .hammer_is_striking = false,
        .key_is_striking = false,
        .note_on_sent = false,
        .note_off_sent = false,
    };
}

void keyhammer_update(Key_Hammer* kh, float pos, float dt) {
    kh->key_pos_prev = kh->key_pos;
    kh->key_pos = pos;
    kh->key_velocity = (kh->key_pos - kh->key_pos_prev) / dt;

    float original_speed = kh->hammer_velocity;
    kh->hammer_velocity -= kh->gravity * dt;
    kh->hammer_pos += (original_speed + kh->hammer_velocity) * dt / 2;

    if (kh->hammer_pos < kh->key_pos) {
        kh->hammer_pos = kh->key_pos;
        if (kh->hammer_velocity < kh->key_velocity) {
            kh->hammer_velocity = kh->key_velocity;
        }
    }

    kh->key_is_striking = kh->key_pos >= kh->key_strike_distance;
    kh->hammer_is_striking = kh->hammer_pos > kh->hammer_travel;

    if (kh->hammer_is_striking) {
        kh->hammer_velocity = -kh->hammer_velocity;
        kh->hammer_pos = kh->hammer_travel;
    }
}
