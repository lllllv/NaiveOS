#include "display.h"

void kern()
{
    //*((int*)0xf00b8000)=0x07690748;
    for(int i = 0; i < 2500; i++)
        putc('0' + (i % 10), 0x07);
    spin: 
        goto spin;
}



