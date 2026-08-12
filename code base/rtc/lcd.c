
#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include "lcd.h"
#include <stdio.h>
#include <stdarg.h>

#define LCD_DATA_PORT PORTC
#define LCD_CTRL_PORT PORTD
#define LCD_RS PD4
#define LCD_EN PD5

static void lcd_send_nibble(uint8_t nibble) {
    LCD_DATA_PORT = (LCD_DATA_PORT & 0xF0) | (nibble & 0x0F);
    LCD_CTRL_PORT |= (1 << LCD_EN);
    _delay_us(1);
    LCD_CTRL_PORT &= ~(1 << LCD_EN);
    _delay_us(100);
}

static void lcd_send_byte(uint8_t byte, uint8_t is_data) {
    if (is_data)
        LCD_CTRL_PORT |= (1 << LCD_RS);
    else
        LCD_CTRL_PORT &= ~(1 << LCD_RS);

    lcd_send_nibble(byte >> 4);
    lcd_send_nibble(byte & 0x0F);
}

void lcd_init(void) {
    DDRC |= 0x0F;
    DDRD |= (1 << LCD_RS) | (1 << LCD_EN);
    _delay_ms(20);
    lcd_send_nibble(0x03);
    _delay_ms(5);
    lcd_send_nibble(0x03);
    _delay_us(100);
    lcd_send_nibble(0x03);
    lcd_send_nibble(0x02);

    lcd_send_byte(0x28, 0);
    lcd_send_byte(0x0C, 0);
    lcd_send_byte(0x06, 0);
    lcd_send_byte(0x01, 0);
    _delay_ms(2);
}

void lcd_clear(void) {
    lcd_send_byte(0x01, 0);
    _delay_ms(2);
}

void lcd_goto(uint8_t row, uint8_t col) {
    uint8_t addr = (row == 0) ? 0x80 : 0xC0;
    lcd_send_byte(addr + col, 0);
}

void lcd_puts(const char *str) {
    while (*str) {
        lcd_send_byte(*str++, 1);
    }
}

void lcd_printf(const char *fmt, ...) {
    char buf[16];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    lcd_puts(buf);
}
