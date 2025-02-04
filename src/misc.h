#ifndef MISC_H
#define MISC_H

#include <stdint.h>

#define F_CPU 16000000

typedef uint8_t  u3;
typedef uint8_t  u4;
typedef uint8_t  u7;
typedef  int8_t  i8;
typedef uint8_t  u8;
typedef uint16_t u10;
typedef uint16_t u14;
typedef uint16_t u16;
typedef  int32_t i32;
typedef uint32_t u32;

// https://www.arduino.cc/reference/en/language/functions/math/map/
i32 map(i32 x, i32 in_min, i32 in_max, i32 out_min, i32 out_max);

int memory_available(void);

#endif
