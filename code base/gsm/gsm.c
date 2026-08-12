#include "gsm.h"
#include <string.h>

static void (*gsm_send_byte)(unsigned char c);
static int  (*gsm_get_byte)(void);

// ---------------- GSM Initialization ----------------
void gsm_init(void (*sendByte_func)(unsigned char c), int (*getByte_func)(void)) {
    gsm_send_byte = sendByte_func;
    gsm_get_byte = getByte_func;

    lcd_clear();
    lcd_puts("GSM Init...");
    _delay_ms(500);

    // Basic AT test
    if (gsm_command_response("AT", "OK", 2000))
        lcd_puts("AT OK");
    else
        lcd_puts("AT Fail");
    _delay_ms(1000);

    // Disable echo
    gsm_command_response("ATE0", "OK", 2000);

    // Set SMS text mode
    gsm_text_sms();
}

// ---------------- Send AT Command ----------------
int gsm_command(char *command) {
    if (!gsm_send_byte) return 0;

    for (int i = 0; command[i]; i++)
        gsm_send_byte(command[i]);
    gsm_send_byte('\n');
    _delay_ms(200);
    return 1;
}

// ---------------- Send AT Command and wait for response ----------------
int gsm_command_response(char *command, char *expected, uint16_t timeout_ms) {
    char line[64];
    uint16_t t = 0;

    gsm_command(command);

    while (t < timeout_ms) {
        if (gsm_readline(line, sizeof(line))) {
            lcd_clear();
            lcd_puts(line);  // debug on LCD
            if (strstr(line, expected)) return 1; // Expected response received
        }
        _delay_ms(50);
        t += 50;
    }

    lcd_clear();
    lcd_puts("No Resp");
    return 0; // timeout
}

// ---------------- Set Text SMS Mode ----------------
void gsm_text_sms(void) {
    gsm_command_response("AT+CMGF=1", "OK", 2000);
}

// ---------------- Send SMS ----------------
int gsm_send_sms(char *number, char *text, int try_num) {
    char line[64];
    char buf[32];

    // Prepare AT+CMGS command
    snprintf(buf, sizeof(buf), "AT+CMGS=\"%s\"", number);
    gsm_command(buf);

    // Wait for ">" prompt from GSM
    uint16_t t = 0;
    while (t < 5000) { // wait 5 sec max
        if (gsm_readline(line, sizeof(line))) {
            lcd_clear();
            lcd_puts(line);
            if (strchr(line, '>')) break; // Ready to accept message
        }
        _delay_ms(50);
        t += 50;
    }

    // Send SMS text
    for (int i = 0; text[i]; i++)
        gsm_send_byte(text[i]);

    gsm_send_byte(26); // CTRL+Z
    _delay_ms(5000);

    // Check for "OK" response after sending
    if (gsm_readline(line, sizeof(line))) {
        lcd_clear();
        lcd_puts(line);
        if (strstr(line, "OK")) return 1; // SMS sent successfully
    }

    lcd_clear();
    lcd_puts("SMS Fail");
    return 0;
}

// ---------------- Flush UART Buffer ----------------
void gsm_flush_buffer(void) {
    if (!gsm_get_byte) return;
    while (gsm_get_byte() != -1);
}

// ---------------- Read line from GSM ----------------
int gsm_readline(char *str, int length) {
    if (!gsm_get_byte) return 0;

    int count = 0;
    int c;
    while ((c = gsm_get_byte()) != -1) {
        if (c == '\r' || c == '\n') {
            if (count > 0) break;
            else continue;
        }
        if (count < length - 1) str[count++] = c;
    }
    str[count] = 0;
    return count > 0;
}

