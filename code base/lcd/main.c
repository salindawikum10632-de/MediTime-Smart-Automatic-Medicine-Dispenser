#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include "lcd.h"

int main(void) {
    lcd_init();        // Initialize LCD
    lcd_clear();       // Clear screen

    lcd_goto(0,0);     // Row 0, Col 0
    lcd_puts("Hello!");  

    lcd_goto(1,0);     // Row 1, Col 0
    lcd_puts("LCD Working");

    while (1) {
        // Blink message every second
        lcd_clear();
        lcd_goto(0,0);
        lcd_puts("Test OK");
        _delay_ms(1000);

        lcd_clear();
        lcd_goto(0,0);
        lcd_puts("Counter:");
        for (uint8_t i = 0; i < 10; i++) {
            lcd_goto(1,0);
            char buffer[5];
            itoa(i, buffer, 10);
            lcd_puts(buffer);
            _delay_ms(1000);
        }
    }
}
