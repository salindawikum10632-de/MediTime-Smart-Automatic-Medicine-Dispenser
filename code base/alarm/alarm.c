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

// -------------------- ALARM VARIABLES --------------------
uint8_t alarm1_hour = 12, alarm1_min = 0;
uint8_t alarm2_hour = 12, alarm2_min = 30;

// Mode states
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
    for (uint8_t i=0;i<8;i++) {
        if(data & 0x01) DS1302_PORT |= (1<<DS1302_IO);
        else DS1302_PORT &= ~(1<<DS1302_IO);
        DS1302_PORT |= (1<<DS1302_CLK);
        _delay_us(1);
        DS1302_PORT &= ~(1<<DS1302_CLK);
        _delay_us(1);
        data >>=1;
    }
}

uint8_t ds1302_read_byte(void) {
    uint8_t data=0;
    DS1302_DDR &= ~(1<<DS1302_IO);
    for(uint8_t i=0;i<8;i++){
        data >>=1;
        if(DS1302_PIN & (1<<DS1302_IO)) data|=0x80;
        DS1302_PORT |= (1<<DS1302_CLK);
        _delay_us(1);
        DS1302_PORT &= ~(1<<DS1302_CLK);
        _delay_us(1);
    }
    DS1302_DDR |= (1<<DS1302_IO);
    return data;
}

uint8_t bcd_to_dec(uint8_t bcd){ return ((bcd>>4)*10)+(bcd&0x0F); }
uint8_t dec_to_bcd(uint8_t dec){ return ((dec/10)<<4)|(dec%10); }

void ds1302_write(uint8_t addr,uint8_t data){
    ds1302_start();
    ds1302_write_byte(addr);
    ds1302_write_byte(data);
    ds1302_stop();
}

uint8_t ds1302_read(uint8_t addr){
    uint8_t data;
    ds1302_start();
    ds1302_write_byte(addr|0x01);
    data = ds1302_read_byte();
    ds1302_stop();
    return data;
}

void ds1302_set_time(uint8_t h,uint8_t m,uint8_t s){
    ds1302_write(0x8E,0x00);
    ds1302_write(0x80,dec_to_bcd(s));
    ds1302_write(0x82,dec_to_bcd(m));
    ds1302_write(0x84,dec_to_bcd(h));
    ds1302_write(0x8E,0x80);
}

void ds1302_get_time(uint8_t *h,uint8_t *m,uint8_t *s){
    *s = bcd_to_dec(ds1302_read(0x80));
    *m = bcd_to_dec(ds1302_read(0x82));
    *h = bcd_to_dec(ds1302_read(0x84));
}

// -------------------- BUTTON READ --------------------
uint8_t button_pressed(volatile uint8_t *pin_reg, uint8_t pin, uint8_t *last_state) {
    uint8_t current_state = !(*pin_reg & (1<<pin)); // Active-low
    if(current_state && !(*last_state)){
        _delay_ms(50);
        current_state = !(*pin_reg & (1<<pin));
        if(current_state){
            *last_state = 1;
            return 1;
        }
    } else if(!current_state && *last_state) {
        *last_state = 0;
    }
    return 0;
}

// -------------------- BUZZER --------------------
void buzzer_on(){ PORTB |= (1<<BUZZER); }
void buzzer_off(){ PORTB &= ~(1<<BUZZER); }

// -------------------- MAIN --------------------
int main(void){
    uint8_t hour,min,sec;

    // Init LCD & RTC
    lcd_init();
    lcd_clear();
    ds1302_init();

    // Init buttons & buzzer
    DDRB &= ~(1<<BTN_SET); PORTB |= (1<<BTN_SET);
    DDRC &= ~(1<<BTN_NEXT | 1<<BTN_UP); PORTC |= (1<<BTN_NEXT | 1<<BTN_UP);
    DDRD &= ~(1<<BTN_DOWN); PORTD |= (1<<BTN_DOWN);
    DDRB |= (1<<BUZZER);

    // Set time once (comment after first upload)
    ds1302_set_time(12,0,0);

    while(1){
        ds1302_get_time(&hour,&min,&sec);

        // Handle buttons
        if(button_pressed(&PINB,BTN_SET,&last_btn_set)){
            mode++;
            if(mode>MODE_SET_ALARM2) mode=MODE_NORMAL;
            edit_field=0;
        }
        if(button_pressed(&PINC,BTN_NEXT,&last_btn_next) && mode!=MODE_NORMAL){
            edit_field ^=1;
        }

        // Alarm1 editing
        if(mode==MODE_SET_ALARM1){
            if(button_pressed(&PINC,BTN_UP,&last_btn_up)){
                if(edit_field==0) alarm1_hour=(alarm1_hour+1)%24;
                else alarm1_min=(alarm1_min+1)%60;
            }
            if(button_pressed(&PIND,BTN_DOWN,&last_btn_down)){
                if(edit_field==0) alarm1_hour=(alarm1_hour+23)%24;
                else alarm1_min=(alarm1_min+59)%60;
            }
        }
        // Alarm2 editing
        if(mode==MODE_SET_ALARM2){
            if(button_pressed(&PINC,BTN_UP,&last_btn_up)){
                if(edit_field==0) alarm2_hour=(alarm2_hour+1)%24;
                else alarm2_min=(alarm2_min+1)%60;
            }
            if(button_pressed(&PIND,BTN_DOWN,&last_btn_down)){
                if(edit_field==0) alarm2_hour=(alarm2_hour+23)%24;
                else alarm2_min=(alarm2_min+59)%60;
            }
        }

        // LCD Display
        lcd_goto(0,0);
        lcd_printf("Time %02d:%02d:%02d  ",hour,min,sec);
        lcd_goto(1,0);
        if(mode==MODE_NORMAL)
            lcd_printf("A %02d:%02d B %02d:%02d",alarm1_hour,alarm1_min,alarm2_hour,alarm2_min);
        else if(mode==MODE_SET_ALARM1)
            lcd_printf("Set A %s %02d:%02d",(edit_field==0)?"Hr":"Mn",alarm1_hour,alarm1_min);
        else if(mode==MODE_SET_ALARM2)
            lcd_printf("Set B %s %02d:%02d",(edit_field==0)?"Hr":"Mn",alarm2_hour,alarm2_min);

        // Check alarms
        if(mode==MODE_NORMAL){
            if((hour==alarm1_hour && min==alarm1_min && sec==0) ||
               (hour==alarm2_hour && min==alarm2_min && sec==0)){
                buzzer_on();
                _delay_ms(5000);
                buzzer_off();
            }
        }

        _delay_ms(100);
    }
}
