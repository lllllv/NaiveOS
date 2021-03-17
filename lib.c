#include "lib.h"


static uint16_t* cursor = (uint16_t*)0xb8000;

void putc(char c)
{
    uint16_t t = (0x07 << 8) | c;
    asm volatile("movw %0, (%1)"
        : 
        : "r"(t), "r"(cursor));

    cursor++;
}

void memset(void* p, uint32_t size, int num)
{
    for(uint32_t i = 0; i < size; i++)
        *(int*)p++ = num;
}

void memcpy(void* dst, void* src, uint32_t size)
{
    for(uint32_t i = 0; i < size; i++)
        *(char*)dst++ = *(char*)src++;
}