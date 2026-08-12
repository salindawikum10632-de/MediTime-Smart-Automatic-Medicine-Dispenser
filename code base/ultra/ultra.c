#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include "lcd.h"

#define TRIG PB0
#define ECHO PB1

void ultrasonic_init() {
    DDRB |= (1 << TRIG);   // TRIG as output
    DDRB &= ~(1 << ECHO);  // ECHO as input
}

uint16_t ultrasonic_read() {
    uint16_t count;

    // Send 10us pulse on TRIG
    PORTB &= ~(1 << TRIG);
    _delay_us(2);
    PORTB |= (1 << TRIG);
    _delay_us(10);
    PORTB &= ~(1 << TRIG);

    // Wait for ECHO high
    while (!(PINB & (1 << ECHO)));

    // Count how long ECHO stays high
    count = 0;
    while (PINB & (1 << ECHO)) {
        count++;
        _delay_us(1);
    }

    // Convert to distance (cm)
    return (count / 58);
}

int main(void) {
    uint16_t distance;

    lcd_init();
    ultrasonic_init();

    while (1) {
        distance = ultrasonic_read();

        lcd_clear();
        lcd_goto(0,0);
        lcd_printf("Dist: %d cm", distance);

        _delay_ms(500); // Update every 0.5s
    }
}
