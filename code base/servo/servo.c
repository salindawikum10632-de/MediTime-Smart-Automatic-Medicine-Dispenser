#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>

void servo_init() {
    DDRB |= (1 << PB2); // Set PB2 (OC1B) as output
    ICR1 = 39999; // 20 ms period (50 Hz) at 1 MHz timer clock
    TCCR1A = (1 << COM1B1) | (1 << WGM11); // Clear OC1B on compare match, Fast PWM
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11); // Fast PWM, prescaler 8
}

void servo_angle(uint8_t angle) {
    // Map 0-180 degrees to 500-2400 µs pulse width
    uint32_t pulse = 500 + ((angle * 1900UL) / 180); // 500 µs to 2400 µs
    OCR1B = pulse; // Convert µs to timer ticks (1 µs per tick)
}

int main(void) {
    servo_init();
    while (1) {
        servo_angle(0);   // 0 degrees (~500 µs)
        _delay_ms(1000);
        servo_angle(90);  // 90 degrees (~1450 µs)
        _delay_ms(1000);
        servo_angle(180); // 180 degrees (~2400 µs)
        _delay_ms(1000);
    }
}
