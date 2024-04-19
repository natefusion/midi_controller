#ifndef LCD_H
#define LCD_H

#include "misc.h"

void lcd_init(void);
void lcd_display_clear(void);
void lcd_goto(u8 x, u8 y);
void lcd_printf(const char* fmt, ...);

#endif
