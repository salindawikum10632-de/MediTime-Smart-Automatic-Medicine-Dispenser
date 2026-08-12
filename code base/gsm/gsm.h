#ifndef GSM_H
#define GSM_H

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lcd.h"

// -------------------- GSM TYPES --------------------
typedef struct {
    char body[160];
    char number[20];
} SMS;

// -------------------- GSM FUNCTIONS --------------------

// Initialize GSM module
void gsm_init(void (*sendByte_func)(unsigned char c), int (*getByte_func)(void));

// Send AT command to GSM module
int gsm_command(char *command);

// Set GSM text SMS mode
void gsm_text_sms(void);

// Send SMS
// number: recipient phone number
// text: message content
// try_num: number of attempts
int gsm_send_sms(char *number, char *text, int try_num);

// Flush GSM buffer (clear UART receive)
void gsm_flush_buffer(void);

// Low-level line reader
int gsm_readline(char *str, int length);

#endif

