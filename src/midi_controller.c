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

// 10 millisecond loop period
// which is long enough to do the calculations but short enough to be responsive
#define SCALED_CYCLES_PER_LOOP 20000
#define DELTA_TIME (float)SCALED_CYCLES_PER_LOOP * 8.0f / (float)F_CPU

#define NUM_SENSORS 5

#define BUTTON_LEFT 1
#define BUTTON_MIDDLE 2
#define BUTTON_RIGHT 3

static Note starting_note = Note_C;
static bool simulate_hammer = false;
static bool turn_all_notes_off = false;

void calibrate(Hall_Effect sensors[NUM_SENSORS]) {
    lcd_display_clear();
    lcd_goto(0, 0);
    lcd_printf("%s", "Calibrate?");
    lcd_goto(0, 1);
    lcd_printf("%s", "1 - yes, 2 - no");

    while (1) {
        if (PINB & (1 << BUTTON_LEFT))
            break;

        if (PINB & (1 << BUTTON_MIDDLE))
            return;
    }
    
    for (u8 i = 0; i < NUM_SENSORS; ++i) {
        u16 val = 0;
        while ((PINB & (1 << BUTTON_RIGHT)) == 0) {
            lcd_display_clear();
            lcd_goto(0,0);
            val = adc_read_port(i);
            lcd_printf("#%d max is %d", i, val);
            lcd_goto(0,1);
            lcd_printf("%s", "Press 3");
            _delay_ms(100);
        }

        _delay_ms(1000);

        sensors[i].max_adc = val;

        while ((PINB & (1 << BUTTON_RIGHT)) == 0) {
            lcd_display_clear();
            lcd_goto(0,0);
            val = adc_read_port(i);
            lcd_printf("#%d min is %d", i, val);
            lcd_goto(0,1);
            lcd_printf("%s", "Press 3");
            _delay_ms(100);
        }

        sensors[i].min_adc = val;

        _delay_ms(1000);
    }

    lcd_display_clear();
    lcd_printf("%s", "All Done!");
    _delay_ms(1000);
}

void redraw_lcd(void) {
    lcd_display_clear();

    lcd_goto(0,0);
    lcd_printf("%s %d", note_range_tostring(starting_note), starting_note / 12);
    
    lcd_goto(0,1);
    if(simulate_hammer){
        lcd_printf("%s", "Piano Sim On");
    } else {
        lcd_printf("%s", "Piano Sim Off");
    }
}

void debug(Hall_Effect sensors[NUM_SENSORS]) {
    u8 port = 0;
    u16 val = adc_read_port(port);
    usart_send_char(val & 0x00FF);
    usart_send_char((val & 0xFF00) >> 8);
}
        
int main(void) {
    midi_init();
    adc_init();
    lcd_init();

    Hall_Effect sensors[NUM_SENSORS] = {
        // don't change the second and third arguments please.
        halleffect_make(0, 532, 879, 545, 868),
        halleffect_make(1, 518, 878, 533, 875),
        halleffect_make(2, 527, 879, 541, 649),
        halleffect_make(3, 518, 877, 537, 870),
        halleffect_make(4, 536, 880, 548, 730),
    };

    Key_Hammer keyhammers[NUM_SENSORS] = {
        keyhammer_make(sensors[0].max_distance - sensors[0].min_distance),
        keyhammer_make(sensors[1].max_distance - sensors[1].min_distance),
        keyhammer_make(sensors[2].max_distance - sensors[2].min_distance),
        keyhammer_make(sensors[3].max_distance - sensors[3].min_distance),
        keyhammer_make(sensors[4].max_distance - sensors[4].min_distance),
    };

    // calibration happens BEFORE interrupts are enabled to avoid conflicts
    calibrate(sensors);

    redraw_lcd();

    DDRB &= ~(1 << BUTTON_LEFT);
    DDRB &= ~(1 << BUTTON_MIDDLE);
    DDRB &= ~(1 << BUTTON_RIGHT);
    sei();
    PCICR = 1 << PCIE0;
    PCMSK0 = (1 << BUTTON_LEFT) | (1 << BUTTON_MIDDLE) | (1 << BUTTON_RIGHT);

    TCCR1A = 0;
    TCCR1B = (1 << WGM12) | (1 << CS11); // clk/8 prescaler and CTC mode
    OCR1A = SCALED_CYCLES_PER_LOOP;
    TCNT1 = 0;

    while (1) {
        // only send midi messages in the main loop to keep things tidy
        // I don't think it really matters for interrupts though
        if (turn_all_notes_off) {
            midi_set_controller(Controller_Notes_Off, 0);
            turn_all_notes_off = false;
        }
        
        for (u8 i = 0; i < NUM_SENSORS; ++i) {
            Key_Hammer *kh = &keyhammers[i];
            Hall_Effect *sensor = &sensors[i];
            Note note = starting_note + i;

            float position_mm = (float)halleffect_get_value(sensor, adc_read_port(i));

            keyhammer_update(kh, position_mm, DELTA_TIME);

            if (simulate_hammer) {
                if (kh->hammer_is_striking && !kh->note_on_sent) {
                    i32 min_velocity = 100;
                    i32 max_velocity = 1000;
                    i32 velocity = -kh->hammer_velocity;
                    if (velocity < min_velocity) velocity = min_velocity;
                    if (velocity > max_velocity) velocity = max_velocity;
                    u8 volume = (u8)map(velocity, min_velocity, max_velocity, Volume_ppp, Volume_fff);
                    midi_send_note_on(note, volume);
                    kh->note_on_sent = true;
                }

                if (kh->note_on_sent) {
                    if (kh->hammer_pos < kh->hammer_travel) {
                        midi_send_note_off(note);
                        kh->note_on_sent = false;
                    }
                }
            } else {
                if (kh->key_is_striking && !kh->note_on_sent) {
                    i32 min_velocity = 90;
                    i32 max_velocity = 150;
                    i32 velocity = kh->key_velocity;
                    if (velocity < min_velocity) velocity = min_velocity;
                    if (velocity > max_velocity) velocity = max_velocity;
                    u8 volume = (u8)map(velocity, min_velocity, max_velocity, Volume_ppp, Volume_fff);
                    midi_send_note_on(note, volume);
                    kh->note_on_sent = true;
                }

                if (kh->note_on_sent) {
                    if (kh->key_pos < kh->key_strike_distance) {
                        midi_send_note_off(note);
                        kh->note_on_sent = false;
                    }
                }
            }
        }

        while ((TIFR1 & (1 << OCF1A)) == 0);
        TIFR1 |= 1 << OCF1A;
    }
}

ISR(PCINT0_vect) {
    bool left = PINB & (1 << BUTTON_LEFT);
    bool middle = PINB & (1 << BUTTON_MIDDLE);
    bool right = PINB & (1 << BUTTON_RIGHT);

    if (left) {
        if (starting_note > 0) {
            starting_note -= 1;
            turn_all_notes_off = true;
        }
    } else if (middle) {
        if (starting_note < 0x7F) {
            starting_note += 1;
            turn_all_notes_off = true;
        }
    } else if (right) {
        simulate_hammer = !simulate_hammer;
    }

    redraw_lcd();
}
