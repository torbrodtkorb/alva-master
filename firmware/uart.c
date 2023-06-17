#define __AVR_DEV_LIB_NAME__ avr64db32

#include <uart.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

volatile bool start_flag;
volatile bool abort_flag;

static volatile uint16_t crc;

// Forward declaration
static int uart_put_char (char, FILE*);

static uint16_t update_crc_ccitt(uint16_t crc, uint8_t data) {
  data ^= crc & 0xff;
  data ^= data << 4;
  return ((((uint16_t)data << 8) | ((crc & 0xff00) >> 8)) ^ (uint8_t)(data >> 4) ^ ((uint16_t)data << 3));
}

void uart_init () {
  PORTA.DIRSET = 1 << 0;
  PORTA.DIRCLR = 1 << 1;
  USART0.CTRLB = USART_TXEN_bm | USART_RXEN_bm;
  USART0.CTRLA = USART_RXCIE_bm | USART_TXCIE_bm;
  USART0.BAUD  = 4 * F_CPU / 921600;

  static FILE avr_stdout = FDEV_SETUP_STREAM(uart_put_char, NULL, _FDEV_SETUP_WRITE);
  stdout = &avr_stdout;
}

static int uart_put_char(char c, FILE* file) {
  while ((USART0.STATUS & USART_DREIF_bm) == 0);
  USART0.TXDATAL = c;
  return 0;
}

void uart_send (uint8_t data) {
  crc = update_crc_ccitt(crc, data);
  if ((USART0.STATUS & USART_DREIF_bm) == 0)
    while (1);
  USART0.TXDATAL = data;
}

void uart_reset_crc () {
  crc = 0;
}

void uart_send_crc () {
  USART0.CTRLA &= ~USART_TXCIE_bm;

  while ((USART0.STATUS & USART_DREIF_bm) == 0);
  USART0.TXDATAL = crc;
  while ((USART0.STATUS & USART_DREIF_bm) == 0);
  USART0.TXDATAL = crc >> 8;
  crc = 0;
  _delay_ms(500);
  USART0.CTRLA |= USART_TXCIE_bm;
}

void uart_send_done () {
  while ((USART0.STATUS & USART_DREIF_bm) == 0);
  USART0.TXDATAL = 0;
  _delay_ms(500);
}

void uart_send_abort () {
  _delay_ms(1000);
  while ((USART0.STATUS & USART_DREIF_bm) == 0);
  USART0.TXDATAL = 1;
  _delay_ms(1000);
}

ISR (USART0_TXC_vect) {
  USART0.STATUS = USART_TXCIF_bm;
  uart_transmitted_callback();
}

ISR (USART0_RXC_vect) {
  uint8_t data = USART0.RXDATAL;

  if (data == 0)
    start_flag = true;
  else if (data == 1)
    abort_flag = true;
}
