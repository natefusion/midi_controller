#include "misc.h"

i32 map(i32 x, i32 in_min, i32 in_max, i32 out_min, i32 out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

int memory_available(void) {
    extern int __heap_start,*__brkval;
    int v;
    return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int) __brkval);
}
