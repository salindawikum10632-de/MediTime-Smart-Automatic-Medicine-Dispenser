#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include "lcd.h"

// -------------------- DS1302 DEFINITIONS --------------------
#define DS1302_PORT PORTD
#define DS1302_DDR  DDRD
#define DS1302_PIN  PIND
#define DS1302_CLK  PD2
#define DS1302_IO   PD3
#define DS1302_CE   PD6

// -------------------- BUTTONS & BUZZER --------------------
#define BTN_SET   PB5
#define BTN_NEXT  PC4
#define BTN_UP    PC5
#define BTN_DOWN  PD7
#define BUZZER    PB4

// -------------------- ULTRASONIC --------------------
#define TRIG PB0
#define ECHO PB1

// -------------------- SERVO --------------------
#define SERVO_UPPER PB2   // OC1B (Timer1)
#define SERVO_LOWER PB3   // OC2A (Timer2)

// -------------------- ALARM VARIABLES --------------------
uint8_t alarm1_hour = 12, alarm1_min = 1;
uint8_t alarm2_hour = 12, alarm2_min = 5;

enum { MODE_NORMAL, MODE_SET_ALARM1, MODE_SET_ALARM2 };
uint8_t mode = MODE_NORMAL;
uint8_t edit_field = 0; // 0 = hour, 1 = min

// -------------------- BUTTON STATE TRACKING --------------------
uint8_t last_btn_set = 0;
uint8_t last_btn_next = 0;
uint8_t last_btn_up = 0;
uint8_t last_btn_down = 0;

// -------------------- DS1302 FUNCTIONS --------------------
void ds1302_init(void) {
    DS1302_DDR |= (1 << DS1302_CLK) | (1 << DS1302_CE);
    DS1302_DDR |= (1 << DS1302_IO);
    DS1302_PORT &= ~(1 << DS1302_CLK);
    DS1302_PORT &= ~(1 << DS1302_CE);
}

void ds1302_start(void) { DS1302_PORT |= (1 << DS1302_CE); _delay_us(4); }
void ds1302_stop(void)  { DS1302_PORT &= ~(1 << DS1302_CE); _delay_us(4); }

void ds1302_write_byte(uint8_t data) {
    for (uint8_t i = 0; i < 8; i++) {
        if (data & 0x01) DS1302_PORT |= (1 << DS1302_IO);
        else DS1302_PORT &= ~(1 << DS1302_IO);
        DS1302_PORT |= (1 << DS1302_CLK);
        _delay_us(1);
        DS1302_PORT &= ~(1 << DS1302_CLK);
        _delay_us(1);
        data >>= 1;
    }
}

uint8_t ds1302_read_byte(void) {
    uint8_t data = 0;
    DS1302_DDR &= ~(1 << DS1302_IO);
    for (uint8_t i = 0; i < 8; i++) {
        data >>= 1;
        if (DS1302_PIN & (1 << DS1302_IO)) data |= 0x80;
        DS1302_PORT |= (1 << DS1302_CLK);
        _delay_us(1);
        DS1302_PORT &= ~(1 << DS1302_CLK);
        _delay_us(1);
    }
    DS1302_DDR |= (1 << DS1302_IO);
    return data;
}

uint8_t bcd_to_dec(uint8_t bcd) { return ((bcd >> 4) * 10) + (bcd & 0x0F); }
uint8_t dec_to_bcd(uint8_t dec) { return ((dec / 10) << 4) | (dec % 10); }

void ds1302_write(uint8_t addr, uint8_t data) {
    ds1302_start();
    ds1302_write_byte(addr);
    ds1302_write_byte(data);
    ds1302_stop();
}

uint8_t ds1302_read(uint8_t addr) {
    uint8_t data;
    ds1302_start();
    ds1302_write_byte(addr | 0x01);
    data = ds1302_read_byte();
    ds1302_stop();
    return data;
}

void ds1302_set_time(uint8_t h, uint8_t m, uint8_t s) {
    ds1302_write(0x8E, 0x00);
    ds1302_write(0x80, dec_to_bcd(s));
    ds1302_write(0x82, dec_to_bcd(m));
    ds1302_write(0x84, dec_to_bcd(h));
    ds1302_write(0x8E, 0x80);
}

void ds1302_get_time(uint8_t *h, uint8_t *m, uint8_t *s) {
    *s = bcd_to_dec(ds1302_read(0x80));
    *m = bcd_to_dec(ds1302_read(0x82));
    *h = bcd_to_dec(ds1302_read(0x84));
}

// -------------------- BUTTON READ --------------------
uint8_t button_pressed(volatile uint8_t *pin_reg, uint8_t pin, uint8_t *last_state) {
    uint8_t current_state = !(*pin_reg & (1 << pin)); // Active-low
    if (current_state && !(*last_state)) {
        _delay_ms(60); // debounce
        current_state = !(*pin_reg & (1 << pin));
        if (current_state) {
            *last_state = 1;
            return 1;
        }
    } else if (!current_state && *last_state) {
        *last_state = 0;
    }
    return 0;
}

// -------------------- BUZZER --------------------
void buzzer_on() { PORTB |= (1 << BUZZER); }
void buzzer_off() { PORTB &= ~(1 << BUZZER); }

// -------------------- SERVO UPPER (PB2) --------------------
void servo_upper_init() {
    DDRB |= (1 << SERVO_UPPER);
    ICR1 = 39999; 
    TCCR1A = (1 << COM1B1) | (1 << WGM11);
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11);
}

void servo_upper_angle(uint8_t angle) {
    uint32_t pulse = 500 + ((angle * 1900UL) / 180);
    OCR1B = pulse;
}

// -------------------- SERVO LOWER (PB3) --------------------
void servo_lower_init() {
    DDRB |= (1 << SERVO_LOWER);
    TCCR2A = (1 << COM2A1) | (1 << WGM21) | (1 << WGM20);
    TCCR2B = (1 << CS22) | (1 << CS20);
    OCR2A = 94;
}

void servo_lower_angle(uint8_t angle) {
    uint32_t pulse = 500 + ((angle * 2000UL) / 180);
    OCR2A = pulse / 16;
    
}



// -------------------- ULTRASONIC --------------------
void ultrasonic_init() {
    DDRB |= (1 << TRIG);
    DDRB &= ~(1 << ECHO);
}

uint16_t ultrasonic_read() {
    uint16_t count = 0;
    PORTB &= ~(1 << TRIG);
    _delay_us(2);
    PORTB |= (1 << TRIG);
    _delay_us(10);
    PORTB &= ~(1 << TRIG);
    while (!(PINB & (1 << ECHO)));
    while (PINB & (1 << ECHO)) {
        count++;
        _delay_us(1);
    }
    return (count / 58);
}

// -------------------- MAIN --------------------
int main(void) {
    uint8_t hour, min, sec;

    lcd_init();
    lcd_clear();
    ds1302_init();
    servo_upper_init();
    servo_lower_init();
    ultrasonic_init();
	// **Set initial servo positions**
	servo_upper_angle(0);   // Upper servo to 0° at startup
	servo_lower_angle(0);   // Lower servo to 0° at startup
    DDRB &= ~(1 << BTN_SET); PORTB |= (1 << BTN_SET);
    DDRC &= ~((1 << BTN_NEXT) | (1 << BTN_UP)); PORTC |= (1 << BTN_NEXT) | (1 << BTN_UP);
    DDRD &= ~(1 << BTN_DOWN); PORTD |= (1 << BTN_DOWN);
    DDRB |= (1 << BUZZER);

    ds1302_set_time(12, 00, 0);

    while (1) {
        ds1302_get_time(&hour, &min, &sec);

        if (button_pressed(&PINB, BTN_SET, &last_btn_set)) {
            mode++;
            if (mode > MODE_SET_ALARM2) mode = MODE_NORMAL;
            edit_field = 0;
        }
        if (button_pressed(&PINC, BTN_NEXT, &last_btn_next) && mode != MODE_NORMAL) {
            edit_field ^= 1;
        }
        if (mode == MODE_SET_ALARM1) {
            if (button_pressed(&PINC, BTN_UP, &last_btn_up)) {
                if (edit_field == 0) alarm1_hour = (alarm1_hour + 1) % 24;
                else alarm1_min = (alarm1_min + 1) % 60;
            }
            if (button_pressed(&PIND, BTN_DOWN, &last_btn_down)) {
                if (edit_field == 0) alarm1_hour = (alarm1_hour + 23) % 24;
                else alarm1_min = (alarm1_min + 59) % 60;
            }
        }
        if (mode == MODE_SET_ALARM2) {
            if (button_pressed(&PINC, BTN_UP, &last_btn_up)) {
                if (edit_field == 0) alarm2_hour = (alarm2_hour + 1) % 24;
                else alarm2_min = (alarm2_min + 1) % 60;
            }
            if (button_pressed(&PIND, BTN_DOWN, &last_btn_down)) {
                if (edit_field == 0) alarm2_hour = (alarm2_hour + 23) % 24;
                else alarm2_min = (alarm2_min + 59) % 60;
            }
        }

        lcd_goto(0, 0);
        lcd_printf("Time %02d:%02d:%02d  ", hour, min, sec);
        lcd_goto(1, 0);
        if (mode == MODE_NORMAL)
            lcd_printf("A %02d:%02d B %02d:%02d", alarm1_hour, alarm1_min, alarm2_hour, alarm2_min);
        else if (mode == MODE_SET_ALARM1)
            lcd_printf("Set A %s %02d:%02d", (edit_field == 0) ? "Hr" : "Mn", alarm1_hour, alarm1_min);
        else if (mode == MODE_SET_ALARM2)
            lcd_printf("Set B %s %02d:%02d", (edit_field == 0) ? "Hr" : "Mn", alarm2_hour, alarm2_min);

	// -------------------- ALARM check --------------------
	if (mode == MODE_NORMAL) {
	    // -------- Alarm 1 --------
	    if (hour == alarm1_hour && min == alarm1_min && sec == 0) {

		buzzer_on();
		lcd_goto(2, 0);
		lcd_printf("Alarm1: Waiting");

		uint16_t distance;
		uint16_t timeout = 0;
		uint8_t hand_detected = 0;

		// Wait 3 minutes for hand detection
		while (timeout < 1800) { // 3 min ≈ 1800 * 100ms
		    distance = ultrasonic_read();
		    if (distance > 2 && distance < 15) {
		        hand_detected = 1;
		        servo_lower_angle(180);
		        _delay_ms(2000);
		        servo_lower_angle(0);
		        _delay_ms(2000);
		        servo_upper_angle(180);
		        _delay_ms(2000);
		        servo_upper_angle(0);
		        lcd_goto(2, 0);
		        lcd_printf("Hand Detected");
		        break;
		    }
		    _delay_ms(100);
		    timeout++;
		}

		if (!hand_detected) {
		    lcd_goto(2, 0);
		    lcd_printf("No Hand Action");

		    // Immediately do lower + upper actions
		    servo_lower_angle(180);
		    _delay_ms(2000);
		    servo_lower_angle(0);
		    _delay_ms(2000);

		    servo_upper_angle(180);
		    _delay_ms(2000);
		    servo_upper_angle(0);
		}

		buzzer_off();
		lcd_goto(2, 0);
		lcd_printf("                ");
	    }

	    // -------- Alarm 2 --------
	    else if (hour == alarm2_hour && min == alarm2_min && sec == 0) {
		buzzer_on();
		lcd_goto(2, 0);
		lcd_printf("Alarm2: Waiting");

		uint16_t distance;
		uint16_t timeout = 0;
		uint8_t hand_detected = 0;

		// Wait 3 minutes
		while (timeout < 1800) {
		    distance = ultrasonic_read();
		    if (distance > 2 && distance < 15) {
		        hand_detected = 1;
		        lcd_goto(2, 0);
		        lcd_printf("Hand Detected");
		        break;
		    }
		    _delay_ms(100);
		    timeout++;
		}

		// After 3 minutes → stop buzzer either way
		buzzer_off();
		if (!hand_detected) {
		    lcd_goto(2, 0);
		    lcd_printf("No Hand -> Off");
		}
		_delay_ms(2000);
		lcd_goto(2, 0);
		lcd_printf("                ");
	    }
	}


        _delay_ms(100);
    }
}

