#include "types.h"

inline void outw(uint16_t port, uint16_t data)
{
    asm volatile("outw %0, %1"
        : 
        : "a"(data), "d"(port));
}

inline void outb(uint16_t port, uint8_t data)
{
    asm volatile("outb %0, %1"
        : 
        : "a"(data), "d"(port));
}

inline uint16_t inw(uint16_t port)
{
    uint16_t result;
    asm volatile("inw %1, %0"
        : "=a"(result)
        : "d"(port));

    return result;
}

inline uint8_t inb(uint16_t port)
{
    uint8_t result;
    asm volatile("inb %1, %0"
        : "=a"(result)
        : "d"(port));

    return result;
}