#include "elf.h"

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

static inline void wait_disk()
{
    while((inb(0x1f7) & 0xC0) != 0x40);
}

void memset(void* p, int c, uint32_t size)
{
    for(uint32_t i = 0; i < size; i++)
        *(char*)p++ = c;
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

    wait_disk();
    
    //  每次从端口读取1个word
    for(int i = 0; i < 512/2; i++)
    {
        uint16_t result = inw(0x1f0);
        *addr = result;
        addr++;
    }
}



void kernel_loader()
{
    struct Elf* kern_header = (struct Elf*)(0x10000);


    read_sector((uint16_t*)kern_header, 1);

    if(kern_header->e_magic != 0x464C457F)
        goto spin;

    struct Proghdr* proghdr = (struct Proghdr*)(kern_header->e_phoff + 0x10000);

    for(int i = 0; i < kern_header->e_phnum; i++)
    {
        if(proghdr[i].p_type != ELF_PROG_LOAD)
            continue;

        memset((void*)proghdr[i].p_pa, 0, proghdr[i].p_memsz);

        char* dest = (char*)proghdr[i].p_pa - proghdr[i].p_offset % 512;

      
        uint32_t start_sector = proghdr[i].p_offset / 512 + 1;


        uint32_t sector_num = (((proghdr[i].p_offset % 512 + proghdr[i].p_filesz) % 512) == 0) ? 
                                ((proghdr[i].p_offset % 512 + proghdr[i].p_filesz) / 512) : 
                                ((proghdr[i].p_offset % 512 + proghdr[i].p_filesz) / 512) + 1;

        for(int j = 0; j < sector_num; j++)
        {
            read_sector((uint16_t*)dest, start_sector + j);
            dest += 512;
        }
    }

    ((void (*)(void))(kern_header->e_entry))();

    spin: 
        goto spin;

}



