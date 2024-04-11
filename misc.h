#ifndef MISC_H
#define MISC_H

#if __STDC_VERSION__ < 202000LL
#error "I am using features from C23. Get a better compiler, or set the right compiler flags (I am using -std=c2x)"
#endif

#define F_CPU 16000000
/* #define mmcu __AVR_ATmega328P__ */

typedef unsigned char u8;
/* typedef unsigned char bool; */
typedef unsigned int u16;

#endif
