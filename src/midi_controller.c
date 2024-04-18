#include "misc.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdbool.h>

#include "lcd.h"
#include "midi.h"
#include "keyboard.h"
#include "adc.h"
#include "halleffect.h"

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

    Hall_Effect sensors[NUM_SENSORS] = {
        { .port = 0,
          .operational_min_adc = 532,
          .operational_max_adc = 879,
          .min_adc = 546,
          .max_adc = 813,
          .max_distance = halleffect_distance_curve(0, 1),
          //                                           v-----v-- ENSURE THESE NUMBERS ARE CHANGED IF min_adc OR max_adc CHANGE
          .min_distance = halleffect_distance_curve(1, 813 - 546 + 1),
        },
        { .port = 1,
          .operational_min_adc = 518,
          .operational_max_adc = 878,
          .min_adc = 532,
          .max_adc = 878,
          .max_distance = halleffect_distance_curve(1, 1),
          //                                           v-----v-- ENSURE THESE NUMBERS ARE CHANGED IF min_adc OR max_adc CHANGE
          .min_distance = halleffect_distance_curve(1, 878 - 532 + 1),
        },
        { .port = 2,
          .operational_min_adc = 527,
          .operational_max_adc = 879,
          .min_adc = 543,
          .max_adc = 752,
          .max_distance = halleffect_distance_curve(2, 1),
          //                                           v-----v-- ENSURE THESE NUMBERS ARE CHANGED IF min_adc OR max_adc CHANGE
          .min_distance = halleffect_distance_curve(1, 752 - 543 + 1),
        },
        { .port = 3,
          .operational_min_adc = 518,
          .operational_max_adc = 877,
          .min_adc = 534,
          .max_adc = 843,
          .max_distance = halleffect_distance_curve(3, 1),
          //                                           v-----v-- ENSURE THESE NUMBERS ARE CHANGED IF min_adc OR max_adc CHANGE
          .min_distance = halleffect_distance_curve(1, 843 - 534 + 1),
        },
        { .port = 4,
          .operational_min_adc = 536,
          .operational_max_adc = 880,
          .min_adc = 548,
          .max_adc = 791,
          .max_distance = halleffect_distance_curve(4, 1),
          //                                           v-----v-- ENSURE THESE NUMBERS ARE CHANGED IF min_adc OR max_adc CHANGE
          .min_distance = halleffect_distance_curve(1, 791 - 548 + 1),
        },
    };

    /* calibrate(hammers); */
    
    Key_Hammer keyhammers[NUM_SENSORS] = {
        keyhammer_make(sensors[0].max_distance - sensors[0].min_distance),
        keyhammer_make(sensors[0].max_distance - sensors[0].min_distance),
        keyhammer_make(sensors[0].max_distance - sensors[0].min_distance),
        keyhammer_make(sensors[0].max_distance - sensors[0].min_distance),
        keyhammer_make(sensors[0].max_distance - sensors[0].min_distance),
    };

    TCCR1A = 0;
    TCCR1B = (1 << WGM12) | (1 << CS11); // clk/8 prescaler and CTC mode
    OCR1A = SCALED_CYCLES_PER_LOOP;
    TCNT1 = 0;

    Note starting_note = Note_C;
    Instrument instrument = Acoustic_Grand_Piano;
    bool simulate_hammer = false;

    // these numbers were hand picked arbitrarily
    i32 min_velocity = 0;
    i32 max_velocity = 1000;

    midi_set_instrument(instrument);

    while (1) {
        for (u8 i = 0; i < NUM_SENSORS; ++i) {
            Key_Hammer *kh = &keyhammers[i];
            Hall_Effect *sensor = &sensors[i];

            float position_mm = (float)halleffect_get_value(sensor, adc_read_port(i));

            keyhammer_update(kh, position_mm, DELTA_TIME);

            if (simulate_hammer) {
                if (kh->hammer_is_striking) {
                    i32 velocity = -kh->hammer_velocity;
                    if (velocity < min_velocity) velocity = min_velocity;
                    if (velocity > max_velocity) velocity = max_velocity;
                    u8 volume = (u8)map(velocity, min_velocity, max_velocity, Volume_pppp, Volume_ffff);

                    midi_send_note_on(starting_note + i, volume);
                    kh->note_off_sent = false;
                } else if (!kh->note_off_sent) {
                    midi_send_note_off(starting_note + i);
                    kh->note_off_sent = true;
                }
            } else {
                // this still needs some work. 
                if (kh->key_is_striking) {
                    i32 velocity = -kh->key_velocity;
                    if (velocity < min_velocity) velocity = min_velocity;
                    if (velocity > max_velocity) velocity = max_velocity;
                    u8 volume = (u8)map(velocity, min_velocity, max_velocity, Volume_pppp, Volume_ffff);

                    midi_send_note_on(starting_note + i, volume);
                    kh->note_off_sent = false;
                } else if (!kh->note_off_sent) {
                    midi_send_note_off(starting_note + i);
                    kh->note_off_sent = true;
                }
            }
        }

        while ((TIFR1 & (1 << OCF1A)) == 0);
        TIFR1 |= 1 << OCF1A;
    }
}
