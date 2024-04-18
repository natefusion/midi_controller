#include "lcd.h"
#include <stdarg.h>
#include <util/delay.h>
#include <avr/io.h>
#include <stdio.h>

#define LCD_EN 2
#define LCD_RS 3
#define DATA PORTD
#define CTRL PORTD

void lcd_enable(void) {
  CTRL |= 1 << LCD_EN;
}

void lcd_disable(void) {
  CTRL &= ~(1 << LCD_EN);
}

void poke_enable(void) {
  lcd_enable();
  _delay_us(1);
  lcd_disable();
  _delay_us(100);
}

void cmd_mode(void) {
  CTRL &= ~(1 << LCD_RS);
}

void data_mode(void) {
  CTRL |= 1 << LCD_RS;
}

// 4 bit mode
void send_data_hi(u8 x) {
  DATA = (DATA & 0x0F) | (x & 0xF0);
}

// 4 bit mode
void send_data_lo(u8 x) {
  DATA = (DATA & 0x0F) | (x << 4);
}

void lcd_cmd(Lcd_Cmd cmd) {
	cmd_mode();
  send_data_hi(cmd);
  poke_enable();

  send_data_lo(cmd);
  poke_enable();
}

void lcd_data(u8 data) {
	data_mode();
  send_data_hi(data);
  poke_enable();

  send_data_lo(data);
  poke_enable();
}

void lcd_init(void) {
  DDRD = 0xFF;
  lcd_disable();
  lcd_cmd(0x33); // enable 4 bit mode
  lcd_cmd(0x32); // enable 4 bit mode part 2
  lcd_cmd(0x28); // enable 5x7 display for 4 bit mode
  lcd_cmd(Set_Display_On_Cursor_Off);
  lcd_cmd(Display_Clear);
  _delay_us(2000);
}

void lcd_goto(u8 x, u8 y) {
  Lcd_Cmd first_char_adr[] = { 
    [0] = Cursor_Line_One, 
    [1] = Cursor_Line_Two,
  };
  
  lcd_cmd(first_char_adr[y] + x);
  _delay_us(100);
}

void lcd_printf(const char* fmt, ...) {
    char buf[16];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, 16, fmt, args);
    va_end(args);

    u8 i = 0;
    while (buf[i]) {
        lcd_data(buf[i]);
        ++i;
    }
}

