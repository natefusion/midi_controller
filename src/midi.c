#include "misc.h"

#include <avr/io.h>
#include <stdarg.h>
#include <stdio.h>

#include "midi.h"

#define INSTRUMENT_CHANNEL 0
#define DRUM_CHANNEL 9

void usart_send_char(u8 c) {
  while (!(UCSR0A & (1 << UDRE0)));
  UDR0 = c;
}

void usart_printf(const char* fmt, ...) {
    static char buf[64];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, 64, fmt, args);
    va_end(args);

    u16 i = 0;
    while (buf[i]) {
        usart_send_char(buf[i]);
        ++i;
    }
}

char *note_range_tostring(Note note) {
    switch ((note % 12) + Note_C) {
    case Note_C:  return "C,C#,D,Eb,E";
    case Note_CS: return "C#,D,Eb,E,F";
    case Note_D:  return "D,Eb,E,F,F#";
    case Note_Eb: return "Eb,E,F,F#,G";
    case Note_E:  return "E,F,F#,G,Ab";
    case Note_F:  return "F,F#,G,Ab,A";
    case Note_FS: return "F#,G,Ab,A,Bb";
    case Note_G:  return "G,Ab,A,Bb,B";
    case Note_Ab: return "Ab,A,Bb,B,C";
    case Note_A:  return "A,Bb,B,C,C#";
    case Note_Bb: return "Bb,B,C,C#,D";
    case Note_B:  return "B,C,C#,D,Eb";
    default: return "";
    }
}

void midi_init(void) {
    UCSR0B = 1 << TXEN0;
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8 bit data
    UBRR0L = 8; // 57600 Baud changed to 115000
}

void midi_send_note_on(Note note, Volume vol) {
  usart_send_char(Status_Note_On | INSTRUMENT_CHANNEL);
  usart_send_char(note);
  usart_send_char(vol);
}

void midi_send_note_off(Note note) {
  usart_send_char(Status_Note_Off | INSTRUMENT_CHANNEL);
  usart_send_char(note);
  usart_send_char(0);
}

void midi_send_drum_on(Instrument drum, Volume vol) {
  usart_send_char(Status_Note_On | DRUM_CHANNEL);
  usart_send_char(drum);
  usart_send_char(vol);
}

void midi_send_drum_off(Instrument drum) {
  usart_send_char(Status_Note_Off | DRUM_CHANNEL);
  usart_send_char(drum);
  usart_send_char(0);
}

void midi_set_pitch_bend(u14 value) {
  usart_send_char(Status_Pitch_Bend | INSTRUMENT_CHANNEL);

  u8 lsb = value & 0x007F;
  u8 msb = (value & 0x7F00) >> 8;
  usart_send_char(lsb);
  usart_send_char(msb);
}

void midi_set_pressure(Note note, u7 value) {
    usart_send_char(Status_Pressure | INSTRUMENT_CHANNEL);
    usart_send_char(value);
}

void midi_set_instrument(Instrument i) {
  usart_send_char(Status_Instrument_Set | INSTRUMENT_CHANNEL);
  usart_send_char(i);
}

void midi_set_controller(Controller c, u7 value) {
  usart_send_char(Status_Controller_Set | INSTRUMENT_CHANNEL);
  usart_send_char(c);
  usart_send_char(value);
}
