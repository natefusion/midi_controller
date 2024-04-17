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
#include <stdbool.h>


#define SCALED_CYCLES_PER_LOOP 20000
#define DELTA_TIME (float)SCALED_CYCLES_PER_LOOP * 8.0f / (float)F_CPU
#define NUM_SENSORS 5

void calibrate(Hall_Effect sensors[NUM_SENSORS]) {
    for (u8 i = 0; i < NUM_SENSORS; ++i) {
        while ((PINB & 1) == 0) {
            usart_printf("%d max %d\n", i, adc_read_port(i));
            _delay_ms(100);
        }

        sensors[i].max_adc = adc_read_port(i);

        _delay_ms(2000);

        while ((PINB & 1) == 0) {
            usart_printf("%d min %d\n", i, adc_read_port(i));
            _delay_ms(100);
        }

        sensors[i].min_adc = adc_read_port(i);

        _delay_ms(2000);
    }

    usart_printf("%s\n", "All Done!");
}

int main(void) {
    midi_init();
    adc_init();

    /* float min = 548.0f;//175.0f; */
    /* float max = 800.0f;//520.0f; */

    Hall_Effect sensors[NUM_SENSORS] = {
        { .port = 0, .min_adc = 546, .max_adc = 813, },
        { .port = 1, .min_adc = 532, .max_adc = 880, },
        { .port = 2, .min_adc = 543, .max_adc = 752, },
        { .port = 3, .min_adc = 534, .max_adc = 843, },
        { .port = 4, .min_adc = 548, .max_adc = 791, },
    };
    
    Key_Hammer keyhammers[NUM_SENSORS] = {
        keyhammer_make(),
        keyhammer_make(),
        keyhammer_make(),
        keyhammer_make(),
        keyhammer_make(),
    };

    /* calibrate(hammers); */

    TCCR1A = 0;
    TCCR1B = (1 << WGM12) | (1 << CS11); // clk/8 prescaler
    OCR1A = SCALED_CYCLES_PER_LOOP;
    TCNT1 = 0;

    Note starting_note = Note_C;
    Instrument instrument = Acoustic_Grand_Piano;
    bool simulate_hammer = false;

    // these numbers were hand picked
    i32 min_velocity = 0;
    i32 max_velocity = 200;

    /* midi_set_instrument(instrument); */
    u8 a = 4;
    bool button_down = false;
    while (1) {
        u8 i = 4;
        for (u8 i = 0; i < NUM_SENSORS; ++i) {
            Key_Hammer *kh = &keyhammers[i];
            Hall_Effect *sensor = &sensors[i];

            u16 raw_adc = adc_read_port(i);
            
            // find more accurate value to replace 12.0f (hint: should be based on sensor measurements)
            float position_mm = (float)halleffect_get_value(sensor, raw_adc);

            keyhammer_update(kh, position_mm, DELTA_TIME);

            if (simulate_hammer) {
                if (kh->hammer_is_striking) {
                    // maybe remove the negative sign. Does it change the result?
                    i32 velocity = (i32)(fabs(kh->hammer_velocity));
                    u8 volume = (u8)map(velocity, min_velocity, max_velocity, Volume_pppp, Volume_ffff);

                    midi_send_note_on(starting_note + i, volume);
                }
            } else {
                if (kh->key_is_striking) {
                    i32 velocity = (i32)(-kh->key_velocity);
                    if (velocity < min_velocity) velocity = min_velocity;
                    if (velocity > max_velocity) velocity = max_velocity;

                    u8 volume = (u8)map(velocity, min_velocity, max_velocity, Volume_pppp, Volume_ffff);

                    if (i == a)
                        usart_printf("velocity: %u", velocity);
                    midi_send_note_on(starting_note + i, volume);
                }
            }
        }

        while ((TIFR1 & (1 << OCF1A)) == 0);
        TIFR1 |= 1 << OCF1A;
    }
}
