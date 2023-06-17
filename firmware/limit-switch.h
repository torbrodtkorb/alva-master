#ifndef LIMIT_SWITCH_H
#define LIMIT_SWITCH_H

#include <stdbool.h>

void limit_switch_init ();
bool limit_switch_pressed ();
bool limit_switch_not_pressed ();

#endif