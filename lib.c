#include "lib.h"

void memset(void* p, int c, uint32_t size)
{
    for(uint32_t i = 0; i < size; i++)
        *(char*)p++ = c;
}

void memcpy(void* dst, void* src, uint32_t size)
{
    for(uint32_t i = 0; i < size; i++)
        *(char*)dst++ = *(char*)src++;
}