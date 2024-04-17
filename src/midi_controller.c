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
#define DELTA_TIME (float)SCALED_CYCLES_PER_LOOP * 8.0f / (float)F_CPU

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

    float min = 548.0f;//175.0f;
    float max = 800.0f;//520.0f;
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

    /* u8 piano_mode = 1; */

    while (1) {
        for (u8 i = 0; i < 5; ++i) {
            float val = (float)halleffect_get_value(i, adc_read_port(i));

            /* if (key_update(&hammers[i].key, val,delta_time)) { */
            /*     float the_org = -hammers[i].key.speed; */
            /*     i32 casted = (i32)the_org; */
            /*     if (casted < 10) casted = 10; */
            /*     else if (casted > 200) casted = 200; */
            /*     i32 the_map = map(casted, 0, 200, 8, 127); */
            /*     if (!aaa) { */
            /*         /\* usart_printf("PING on %u with volume: %u and velocity: %f\n", i, (u8)the_map, the_org); *\/ */
            /*         midi_send_note_on(Note_C + i, (u8)the_map); */
            /*         aaa = 1; */
            /*     } */
            /* } else { */
            /*     midi_send_note_off(Note_C + i); */
            /*     aaa = 0; */
            /* } */
            if (hammer_update(&hammers[i], 12.0f-val, DELTA_TIME)) {
                float the_org = -hammers[i].speed;
                i32 casted = (i32)the_org;
                if (casted < 10) casted = 10;
                else if (casted > 200) casted = 200;
                i32 the_map = map(casted, 0, 200, 8, 127);
                if (!hammers[i].note_sent) {
                    midi_send_note_on(Note_C + i, (u8)the_map);
                    hammers[i].note_sent = 1;
                }

                /* usart_printf("PING on %u with volume: %u and velocity: %f\n", i, (u8)the_map, the_org); */
            } else {
                if (hammers[i].note_sent) {
                    midi_send_note_off(Note_C + i);
                    hammers[i].note_sent = 0;
                }
            }
        }

        while ((TIFR1 & (1 << OCF1A)) == 0);
        TIFR1 |= 1 << OCF1A;
    }
}
