#include "display.h"
#include "lib.h"


void putc(char content, uint8_t attribute)
{
    cursor++;
    if((uint32_t)cursor > (0xf00b8000 + 4000))
    {
        void* temp = cursor;
        memcpy((void*)0xf00b8000, (void*)(0xf00b8000 + 80 * 2), 80 * 24 * 2);
        cursor -= 80;
        memset(cursor, 0, 80 * 2);
    }
    *cursor = (content | attribute << 8);
    //cursor++;
}

void printf(char* fmt, ...)
{
    
}

int sum(int num,...)
{
 
    va_list valist;
    int r = 0;
    int i;
 
    /* 为 num 个参数初始化 valist */
    va_start(valist, num);
 
    /* 访问所有赋给 valist 的参数 */
    for (i = 0; i < num; i++)
    {
       r += va_arg(valist, int);
    }
    /* 清理为 valist 保留的内存 */
    va_end(valist);
 
    return r;
}