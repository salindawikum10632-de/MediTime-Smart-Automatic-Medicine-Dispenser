#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>

// PB2 = OC1B hardware PWM
// PB3 = software PWM via Timer1 ISR
#define SERVO_HW_PIN PB2  // hardware PWM
#define SERVO_SW_PIN PB3  // software PWM

volatile uint16_t servo_sw_ticks = 1000;  // PB3 pulse width in µs

ISR(TIMER1_COMPA_vect) {
    // Start of frame → PB3 HIGH
    PORTB |= (1 << SERVO_SW_PIN);
}

ISR(TIMER1_COMPB_vect) {
    // After servo_sw_ticks → PB3 LOW
    PORTB &= ~(1 << SERVO_SW_PIN);
}

void servo_init(void) {
    // PB2 hardware PWM, PB3 software PWM
    DDRB |= (1 << SERVO_HW_PIN) | (1 << SERVO_SW_PIN);

    // Timer1 mode 14 Fast PWM, TOP = ICR1
    ICR1 = 20000; // 20ms period
    TCCR1A = (1 << COM1B1) | (1 << WGM11); 
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11); // prescaler 8

    // PB2 default 1ms pulse
    OCR1B = 1000;

    // PB3 software PWM
    OCR1A = 1;  // start of frame for ISR
    TIMSK1 = (1 << OCIE1A) | (1 << OCIE1B);  // enable ISRs

    sei();
}

void servo_pb2_angle(uint8_t angle) {
    uint16_t us = 500 + ((uint32_t)angle * 1900UL) / 180UL;
    OCR1B = us; // hardware PWM
}

void servo_pb3_angle(uint8_t angle) {
    servo_sw_ticks = 500 + ((uint32_t)angle * 1900UL) / 180UL; // software PWM
}

int main(void) {
    servo_init();

    while (1) {
        // Test sequence
        servo_pb2_angle(0);
        //servo_pb3_angle(180);
        _delay_ms(1000);

        servo_pb2_angle(90);
        //servo_pb3_angle(90);
        _delay_ms(1000);

        servo_pb2_angle(180);
        //servo_pb3_angle(0);
        _delay_ms(1000);
    }
}
