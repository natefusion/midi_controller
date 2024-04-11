#ifndef LCD_H
#define LCD_H

#include "misc.h"

typedef enum Lcd_Cmd : u8 {
  Display_Clear = 0x1,
  Cursor_Return = 0x2,
  Cursor_Left = 0x4,
  Cursor_Right = 0x6,
  Display_Right = 0x5,
  Display_Left = 0x7,
  Set_Display_Off_Cursor_Off = 0x8,
  Set_Display_Off_Cursor_On = 0xA,
  Set_Display_On_Cursor_Off = 0xC,
  Set_Display_On_Cursor_On = 0xE,
  Set_Display_On_Cursor_Blink = 0xF,
  Cursor_Pos_Left = 0x10,
  Cursor_Pos_Right = 0x14,
  Display_Shift_Left = 0x18,
  Display_Shift_Right = 0x1C,
  Cursor_Line_One = 0x80,
  Cursor_Line_Two = 0xC0,
} Lcd_Cmd;

void lcd_init(void);
void lcd_goto(u8 x, u8 y);
void lcd_cmd(Lcd_Cmd cmd);
void lcd_printf(const char* fmt, ...);

#endif
