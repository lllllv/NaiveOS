#include "elf.h"
#include "wrapper.h"




static inline void wait_disk()
{
    while((inb(0x1f7) & 0xC0) != 0x40);
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

void read_sector(uint16_t* addr, uint32_t sectno)
{
    wait_disk();

    outb(0x1f1, 0);
    outb(0x1f2, 1);
    outb(0x1f3, sectno);
    outb(0x1f4, sectno >> 8);
    outb(0x1f5, sectno >> 16);
    outb(0x1f6, (sectno >> 24) | 0xe0);
    outb(0x1f7, 0x20);

    //wait_for_read();
    for(int i = 0; i < 1000; i++);
    wait_disk();
    
    //  每次从端口读取1个word
    for(int i = 0; i < 512/2; i++)
    {
        for(int i = 0; i < 100; i++);
        uint16_t result = inw(0x1f0);
        *addr = result;
        addr ++;
    }
}



void kernel_loader()
{
    struct Elf* kern_header = (struct Elf*)(0x10000);

    if(kern_header->e_magic != 0x464C457F)
        goto wrong_elf;

    
    struct Proghdr* proghdr = (struct Proghdr*)(kern_header->e_phoff + 0x10000);

    for(int i = 0; i < kern_header->e_phnum; i++)
    {
        if(proghdr[i].p_type != ELF_PROG_LOAD)
            continue;

        memset((void*)proghdr[i].p_pa, proghdr[i].p_memsz, 0);

        char* dest = (char*)proghdr[i].p_pa - proghdr[i].p_offset % 512;

        uint32_t start_sector = proghdr[i].p_offset / 512 + 1;

        uint32_t sector_num = (((proghdr[i].p_offset % 512 + proghdr[i].p_filesz) % 512) == 0) ? 
                                ((proghdr[i].p_offset % 512 + proghdr[i].p_filesz) / 512) : 
                                ((proghdr[i].p_offset % 512 + proghdr[i].p_filesz) / 512) + 1;

        for(int j = 0; j < sector_num; j++)
            read_sector((uint16_t*)dest, start_sector + j);

    }



    ((void (*)(void))(kern_header->e_entry))();


    // *((int*)0xb8000)=0x07690748;    
    


    wrong_elf: 
        *((int*)0xb8000)=0x07690748;

    spin: 
        goto spin;

}



