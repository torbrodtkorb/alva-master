#ifndef ANALOG_H
#define ANALOG_H

#include <stdint.h>

void analog_init ();
void analog_start ();
void analog_stop ();
void analog_callback (uint16_t result);

#endif