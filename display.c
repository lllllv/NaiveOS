#include "display.h"
#include "lib.h"

static uint16_t* cursor = (uint16_t*)0xf00b7ffe;


//  每次cursor留在原地
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
}

void clear()
{
    memset((void*)0xf00b8000, 0, 28 * 80);
}


void printf(char* fmt, ...)
{
    va_list valist;

    va_start(valist, fmt);

    char* p = fmt;

    // do NOT support escape character now !!!
    int count = 0;
    while(*p != '\0')
    {
        if(*p == '%')
            count++;
        p++;
    }

    while(*fmt != '\0')
    {
        if(*fmt == '%')
        {
            uint32_t tmp = va_arg(valist, int);
            switch(*(fmt + 1))
            {
                case 'd': 
                {   
                    int num = (int)tmp;
                    if(num < 0)
                    {
                        putc('-', 0x07);
                        num = 0 - num;
                    }
                    int r[12] = {0};
                    int c = 0;
                    while(num != 0)
                    {
                        r[c++] = num % 10;
                        num /= 10;
                    }
                    c = 11;
                    while(r[c] == 0 && c > 0)
                        c--;
                    while(c >= 0)
                        putc(r[c--] + '0', 0x07);
                    break;
                }
                case 'x': 
                {
                    int r[8] = {0};
                    int c = 0;
                    while(tmp != 0)
                    {
                        r[c++] = tmp % 16;
                        tmp /= 16;
                    }
                    c = 7;
                    while(r[c] == 0 && c > 0)
                        c--;
                    while(c >= 0)
                    {
                        if(r[c] > 9)
                            putc('a' + r[c] - 10, 0x07);
                        else 
                            putc(r[c] + '0', 0x07);

                        c--;
                    }
                    break;
                }
                case 's':
                {
                    while(*(char*)tmp != '\0')
                        putc(*(char*)tmp++, 0x07);
                    break;
                }
            }
            fmt += 2;
        }
        else if(*fmt == '\n')
        {
            cursor -= (((uint32_t)cursor - 0xf00b8000) / 2) % 80;
            cursor +=  79;

            
            fmt++;
        }
        else if(*fmt == '\t')
            {cursor += 4;
            fmt++;}
        else
            putc(*fmt++, 0x07);
    }

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