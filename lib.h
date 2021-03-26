#include "types.h"
#include "display.h"
#define NULL 0

typedef char *  va_list; 
#define _ALIGNSIZE(arg)     ((sizeof(arg) + sizeof(int) - 1) & ~(sizeof(int) - 1))
#define va_start(ap,v)      (ap = (va_list)&v + _ALIGNSIZE(v))
#define va_arg(ap,t)        (*(t *)((ap += _ALIGNSIZE(t)) - _ALIGNSIZE(t)))
#define va_end(ap)          (ap=(va_list)0)  


void panic(char*msg);
//void exit(int status);
void memset(void* p, int c, uint32_t size);
void memcpy(void* dst, void* src, uint32_t size);