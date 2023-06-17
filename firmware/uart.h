#ifndef UART_H
#define UART_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

extern volatile bool start_flag;
extern volatile bool abort_flag;

void uart_init ();
void uart_transmitted_callback ();
void uart_send (uint8_t data);
void uart_send_done ();
void uart_send_abort ();
void uart_send_crc ();
void uart_reset_crc ();

#endif