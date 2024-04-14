#include "misc.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include "lcd.h"
#include "midi.h"
#include "keyboard.h"
#include "adc.h"
#include <util/delay.h>

#define SCALED_CYCLES_PER_LOOP 5000

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

    float min = 175.0f;
    float max = 520.0f;
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

    while (1) {
        for (u8 i = 0; i < 5; ++i) {
            float val = (float)adc_read_port(i);
            float delta_time = (float)SCALED_CYCLES_PER_LOOP * 8.0f / (float)F_CPU;
            if (hammer_update(&hammers[i], val, delta_time)) {
                usart_printf("PING on %u\n", i);
            }
        }

        while ((TIFR1 & (1 << OCF1A)) == 0);
        TIFR1 |= 1 << OCF1A;
    }
}
