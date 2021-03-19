
void kern()
{
    *((int*)0xf00b8000)=0x07690748;
    spin: 
        goto spin;
}



