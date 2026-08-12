#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include "lcd.h"

// -------------------- DS1302 PIN DEFINITIONS --------------------
#define DS1302_PORT PORTD
#define DS1302_DDR  DDRD
#define DS1302_PIN  PIND

#define DS1302_CLK  PD2
#define DS1302_IO   PD3
#define DS1302_CE   PD6

// -------------------- DS1302 LOW-LEVEL FUNCTIONS --------------------
void ds1302_init(void) {
    DS1302_DDR |= (1 << DS1302_CLK) | (1 << DS1302_CE); // CLK & CE as output
    DS1302_DDR |= (1 << DS1302_IO);  // Initially set IO as output
    DS1302_PORT &= ~(1 << DS1302_CLK); // CLK low
    DS1302_PORT &= ~(1 << DS1302_CE);  // CE low
}

void ds1302_start(void) {
    DS1302_PORT |= (1 << DS1302_CE);  // CE high
    _delay_us(4);
}

void ds1302_stop(void) {
    DS1302_PORT &= ~(1 << DS1302_CE); // CE low
    _delay_us(4);
}

void ds1302_write_byte(uint8_t data) {
    for (uint8_t i = 0; i < 8; i++) {
        // Write LSB first
        if (data & 0x01) DS1302_PORT |= (1 << DS1302_IO);
        else DS1302_PORT &= ~(1 << DS1302_IO);

        // Pulse clock
        DS1302_PORT |= (1 << DS1302_CLK);
        _delay_us(1);
        DS1302_PORT &= ~(1 << DS1302_CLK);
        _delay_us(1);

        data >>= 1;
    }
}

uint8_t ds1302_read_byte(void) {
    uint8_t data = 0;
    DS1302_DDR &= ~(1 << DS1302_IO); // IO as input

    for (uint8_t i = 0; i < 8; i++) {
        data >>= 1;
        if (DS1302_PIN & (1 << DS1302_IO))
            data |= 0x80;

        // Pulse clock
        DS1302_PORT |= (1 << DS1302_CLK);
        _delay_us(1);
        DS1302_PORT &= ~(1 << DS1302_CLK);
        _delay_us(1);
    }

    DS1302_DDR |= (1 << DS1302_IO); // back to output
    return data;
}

// -------------------- BCD CONVERSION --------------------
uint8_t bcd_to_dec(uint8_t bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

uint8_t dec_to_bcd(uint8_t dec) {
    return ((dec / 10) << 4) | (dec % 10);
}

// -------------------- DS1302 HIGH-LEVEL FUNCTIONS --------------------
void ds1302_write(uint8_t addr, uint8_t data) {
    ds1302_start();
    ds1302_write_byte(addr);
    ds1302_write_byte(data);
    ds1302_stop();
}

uint8_t ds1302_read(uint8_t addr) {
    uint8_t data;
    ds1302_start();
    ds1302_write_byte(addr | 0x01); // Read command
    data = ds1302_read_byte();
    ds1302_stop();
    return data;
}

// -------------------- SET & READ TIME --------------------
void ds1302_set_time(uint8_t hour, uint8_t min, uint8_t sec) {
    // Write protect disable
    ds1302_write(0x8E, 0x00);

    ds1302_write(0x80, dec_to_bcd(sec));
    ds1302_write(0x82, dec_to_bcd(min));
    ds1302_write(0x84, dec_to_bcd(hour));

    // Write protect enable
    ds1302_write(0x8E, 0x80);
}

void ds1302_get_time(uint8_t *hour, uint8_t *min, uint8_t *sec) {
    *sec  = bcd_to_dec(ds1302_read(0x80));
    *min  = bcd_to_dec(ds1302_read(0x82));
    *hour = bcd_to_dec(ds1302_read(0x84));
}

// -------------------- MAIN PROGRAM --------------------
int main(void) {
    uint8_t hour, min, sec;

    lcd_init();
    lcd_clear();
    ds1302_init();

    // Set time once (e.g., 14:45:00) → comment after first upload
    ds1302_set_time(11, 36, 0);

    while (1) {
        ds1302_get_time(&hour, &min, &sec);

        lcd_clear();
        lcd_goto(0,0);
        lcd_printf("Time: %02d:%02d:%02d", hour, min, sec);

        _delay_ms(1000);
    }
}
