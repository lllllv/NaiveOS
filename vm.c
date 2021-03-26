#include "inc.h"
#include "vm.h"
#include "lib.h"
#include "elf.h"
#include "display.h"

static uint32_t free_mem;
struct Page* free_pages;
struct Page* ppage;

uint32_t* kern_pgdir;


uint32_t page2pa(struct Page* p)
{
    uint32_t index = p - ppage;
    return PGSIZE * index;
}

struct Page* pa2page(uint32_t pa)
{
    return ppage + pa - pa % PGSIZE;
}

struct Page* ppage_alloc()
{
    if(free_pages == NULL)
    {
        printf("Error! No more space to allocate");
        return NULL;
    }

    struct Page* t = free_pages;
    free_pages = free_pages->next;

    return t;
}

void page_delete(uint32_t* pgdir, uint32_t va)
{
    if(pgdir[PDX(va)])
    {
        uint32_t* page_table = (pgdir[PDX(va)] & ~0xfff) | 0xf0000000;
        if(page_table[PTX(va)] != 0)
        {
            struct Page* p = pa2page(page_table[PTX(va)] & ~0xfff);
            p->ref_count--;
            if(p->ref_count == 0)
            {
                p->next = free_pages;
                free_pages = p;
            }
        }
    }
}

int memory_map(uint32_t* pgdir, uint32_t va, uint32_t pa, uint32_t size, int perm)
{
    if(va % PGSIZE != 0 || pa % PGSIZE != 0)
        return -1;
    
    size = (size % PGSIZE) == 0 ? size : (size / PGSIZE ) * PGSIZE + PGSIZE;

    for(uint32_t i = va; i < va + size; i += PGSIZE)
    {
        if(pgdir[PDX(i)] == 0)
        {
            struct Page* p = ppage_alloc();
            if(!p)
                panic("memory_map(): No more space for a page table");

            p->ref_count++;
            memset(p, 0, PGSIZE);
            uint32_t tmp = page2pa(p);
            pgdir[PDX(i)] = tmp | PTE_P | perm;
            ((uint32_t*)tmp)[PTX(i)] = pa | PTE_P | perm;

        }   
        else 
        {
            uint32_t* page_table = pgdir[PDX(i)] & ~0xfff;
            if(page_table[PTX(i)] != 0)
                page_delete(kern_pgdir,va);

            page_table[PTX(i)] = pa | PTE_P | perm;
        }

        pa += PGSIZE;
        va += PGSIZE;
    }

}

void vm_init()
{
    struct Elf* kern_header = (struct Elf*)(0x10000);
    struct Proghdr* proghdr = (struct Proghdr*)(kern_header->e_phoff + 0x10000);
    free_mem = 0;

    for(int i = 0; i < kern_header->e_phnum; i++)
    {
        if(proghdr[i].p_type != ELF_PROG_LOAD)
            continue;
        
        if(proghdr[i].p_va > free_mem)
            free_mem = proghdr[i].p_va + proghdr[i].p_memsz;
    }

    if(free_mem % PGSIZE != 0)
        free_mem = (free_mem / PGSIZE) * PGSIZE + PGSIZE;

    ppage_init();


    /*             kern page directory        */
    struct Page* p = ppage_alloc();
    kern_pgdir = (uint32_t*)page2pa(p);
    memset(kern_pgdir, 0, PGSIZE);
    
    memory_map(kern_pgdir, 0xf0000000, 0, 0x10000000, PTE_W | PTE_P);




    
    printf("init done");

    asm volatile("movl %0, %%cr3"
                : 
                :"r"(kern_pgdir));

    uint32_t cr0;
    asm volatile("movl %%cr0, %0"
                :"=r"(cr0)
                :);
    cr0 |= CR0_PE|CR0_PG|CR0_AM|CR0_WP|CR0_NE|CR0_MP;
    cr0 &= ~(CR0_TS|CR0_EM);
    asm volatile("movl %0, %%cr0"
                : 
                :"r"(cr0));
}

void* init_alloc(uint32_t size)
{
    if(size % PGSIZE != 0)
        size = (size / PGSIZE) * PGSIZE + PGSIZE;
    free_mem += size;
    return (void*)(free_mem - size);
}

void ppage_init()
{
    ppage = init_alloc(sizeof(struct Page) * 64 * 1024);
    memset(ppage, 0, sizeof(struct Page) * 64 * 1024);

    uint32_t end = 64 * 1024, i = 0;
    for(; i < 640 * 1024 / PGSIZE; i++)
    {
        ppage[i].ref_count = 0;
        ppage[i].next = &ppage[i + 1];
    }

    uint32_t tmp = i;

    for(; i < 1 * 1024 * 1024 / PGSIZE; i++)
    {
        ppage[i].ref_count = 1;
        ppage[i].next = NULL;
    }


    for(; i < (free_mem & 0x0fffffff) / PGSIZE; i++)
    {
        ppage[i].ref_count = 1;
        ppage[i].next = NULL;
    }

    ppage[tmp].next = &ppage[i];

    for(; i < 0x10000000 / PGSIZE; i++)
    {
        ppage[i].ref_count = 0;
        ppage[i].next = &ppage[i + 1];
    }

    ppage[i].next = NULL;
    free_pages = &ppage[0];
}






