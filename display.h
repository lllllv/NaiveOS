#ifndef DISPLAY_H
#define DISPLAY_H

#include "types.h"


void putc(char content, uint8_t attribute);
void clear();
void printf(char* fmt, ...);

#endif