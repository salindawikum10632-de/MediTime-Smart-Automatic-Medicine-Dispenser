#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include "lcd.h"   // your lcd driver

// ---------- UART FUNCTIONS ----------
void UART_init(unsigned int ubrr) {
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);   // Enable RX and TX
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8-bit data
}

void UART_sendChar(char data) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}

void UART_sendString(const char *str) {
    while (*str) {
        UART_sendChar(*str++);
    }
}

char UART_receiveChar(void) {
    while (!(UCSR0A & (1 << RXC0)));
    return UDR0;
}

// ---------- MAIN ----------
int main(void) {
    // Init LCD
    lcd_init();
    lcd_clear();
    lcd_goto(0,0);
    lcd_puts("GSM Test Start");

    // Init UART (9600 baud @ 8MHz -> UBRR = 51)
    UART_init(51);
    _delay_ms(1000);

    // Send AT command
    UART_sendString("AT\r\n");

    _delay_ms(1000); // wait for reply

    // Check if something came back
    char response[16];
    int i = 0;

    while ((UCSR0A & (1 << RXC0)) && (i < 15)) {
        response[i++] = UART_receiveChar();
    }
    response[i] = '\0';

    lcd_clear();
    lcd_goto(0,0);
    if (i > 0) {
        lcd_puts("GSM OK");
    } else {
        lcd_puts("No Response");
    }

    while (1) {
        // stay here
    }
}
