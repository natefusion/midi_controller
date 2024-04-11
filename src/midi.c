#include <avr/io.h>
#include "midi.h"
#include "misc.h"

void usart_send_char(u8 c) {
  while (!(UCSR0A & (1 << UDRE0)));
  UDR0 = c;
}

void midi_init(void) {
    UCSR0B = 1 << TXEN0;
    UCSR0C = (1 << UPM01) | (1 << UCSZ01) | (1 << UCSZ00);
    UBRR0L = 16; // 1 / (57600 * 16 / F_CPU) - 1;
}

void midi_send_note_on(Note note, Volume vol) {
  u8 channel = 0;
  usart_send_char(Status_Note_On | channel);
  usart_send_char(note);
  usart_send_char(vol);
}

void midi_send_note_off(Note note) {
  u8 channel = 0;
  usart_send_char(Status_Note_Off | channel);
  usart_send_char(note);
  usart_send_char(0);
}

void midi_send_drum_on(Instrument drum, Volume vol) {
  u8 channel = 9;
  usart_send_char(Status_Note_On | channel);
  usart_send_char(drum);
  usart_send_char(vol);
}

void midi_send_drum_off(Instrument drum) {
  u8 channel = 9;
  usart_send_char(Status_Note_Off | channel);
  usart_send_char(drum);
  usart_send_char(0);
}

void midi_set_pitch_bend(u16 value) {
  u8 channel = 0;
  usart_send_char(Status_Pitch_Bend | channel);

  u8 lsb = value & 0x007F;
  u8 msb = (value & 0x7F00) >> 8;
  usart_send_char(lsb);
  usart_send_char(msb);
}

void midi_set_instrument(Instrument i) {
  u8 channel = 0;
  usart_send_char(Status_Instrument_Set | channel);
  usart_send_char(i);
}

void midi_set_controller(Controller c, u8 value) {
  u8 channel = 0;
  usart_send_char(Status_Controller_Set | channel);
  usart_send_char(c | value);
}
