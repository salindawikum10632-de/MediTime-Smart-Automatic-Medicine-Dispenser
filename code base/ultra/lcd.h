
#ifndef LCD_H_
#define LCD_H_

void lcd_init(void);
void lcd_clear(void);
void lcd_goto(uint8_t row, uint8_t col);
void lcd_puts(const char *str);
void lcd_printf(const char *fmt, ...);

#endif /* LCD_H_ */
