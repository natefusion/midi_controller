#include "misc.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include "lcd.h"
#include "midi.h"
#include "keyboard.h"
#include "adc.h"
#include "halleffect.h"
#include <util/delay.h>
#include <stdlib.h>
#define SCALED_CYCLES_PER_LOOP 20000

void calibrate(Hammer hammers[5]) {
    float val = 0.0;
    for (u8 i = 0; i < 5; ++i) {
        while ((PINB & 1) == 0) {
            usart_printf("max for %d is %d\n", i, adc_read_port(i));
            _delay_ms(100);
        }

        val = (float)adc_read_port(i);
        hammers[i].key.pos_max = val;

        _delay_ms(2000);

        while ((PINB & 1) == 0) {
            usart_printf("min for %d is %d\n", i, adc_read_port(i));
            _delay_ms(100);
        }

        val = (float)adc_read_port(i);
        hammers[i].key.pos_max = val;

        _delay_ms(2000);
    }

    usart_printf("%s\n", "All Done!");
}

int main(void) {
    midi_init();
    adc_init();

    float min = 536.0f;//175.0f;
    float max = 872.0f;//520.0f;
    Hammer hammers[5] = {
        hammer_make(min, max),
        hammer_make(min, max),
        hammer_make(min, max),
        hammer_make(min, max),
        hammer_make(min, max),
    };

    /* calibrate(hammers); */

    TCCR1A = 0;
    TCCR1B = (1 << WGM12) | (1 << CS11); // clk/8 prescaler
    OCR1A = SCALED_CYCLES_PER_LOOP;
    TCNT1 = 0;


    /* u16 prev_aaa = 0; */
    u16 aaa = 0;
    while (1) {
        u8 i = 0;
        /* for (u8 i = 0; i < 5; ++i) { */
        aaa = adc_read_port(i);
        /* float val = (float)aaa; */
        /* if (abs((i32)aaa - (i32)prev_aaa) > 2) { */
        usart_printf("mem used: %d\n", memory_available());
        /* usart_printf("val: %u, %f\n", aaa, halleffect_get_value(aaa)); */
        /* } */

            /* float delta_time = (float)SCALED_CYCLES_PER_LOOP * 8.0f / (float)F_CPU; */
            /*         if (hammer_update(&hammers[i], val, delta_time)) { */
            /*     float the_org = -hammers[i].speed; */
            /*     i32 casted = (i32)the_org; */
            /*     if (casted < 4000) casted = 4000; */
            /*     else if (casted > 30000) casted = 30000; */
            /*     i32 the_map = map(casted, 4000, 30000, 8, 127); */
            /*     /\* midi_send_note_off(Note_C + i); *\/ */
            /*     /\* midi_send_note_on(Note_C + i, (u8)the_map); *\/ */
            /*     /\* usart_printf("PING on %u with volume: %u and velocity: %f\n", i, (u8)the_map, the_org); *\/ */
            /* } else { */
            /*     /\* midi_send_note_off(Note_C + i); *\/ */
            /* } */
        /* } */

        while ((TIFR1 & (1 << OCF1A)) == 0);
        TIFR1 |= 1 << OCF1A;

        /* prev_aaa = aaa; */
    }
}
