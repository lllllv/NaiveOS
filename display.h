#include "types.h"



static uint16_t* cursor = (uint16_t*)0xf00b7ffe;
void putc(char content, uint8_t attribute);
void printf(char* fmt, ...);