#include <avr/io.h>
#include <util/delay.h>
#include "lcd.h"
#include "gsm.h"

// Dummy UART functions for testing (replace with actual UART code)
void uart_send_byte(unsigned char c) {
    // send c via USART
}
int uart_get_byte(void) {
    return -1; // no data
}

int main(void) {
    lcd_init();
    lcd_clear();
    lcd_puts("Starting Test");
    _delay_ms(1000);

    gsm_init(uart_send_byte, uart_get_byte);

    // Send test SMS
    gsm_send_sms("+94714636728", "Hello from GSM!", 1);

    while (1) {}
}

