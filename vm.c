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
    //if(index ==0) printf("page2pa: %x", PGSIZE * index);
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
        uint32_t* page_table = (uint32_t*)((pgdir[PDX(va)] & ~0xfff) | 0xf0000000);
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

    uint32_t counter = 0;
    for(uint32_t i = va; i != va + size; i += PGSIZE, pa += PGSIZE)
        *page_entry(pgdir, i) = pa | PTE_P | perm;

    return 0;
}



uint32_t* page_entry(uint32_t* pgdir, uint32_t va)
{
    if(!pgdir[PDX(va)])
    {
        if(va == 0x200000)printf("alloc a ppage.");
        struct Page* p = ppage_alloc();
        if(!p)
            panic("pgentry(): No more space for a page table");

        p->ref_count++;
        memset(KADDR(page2pa(p)), 0, PGSIZE);
        uint32_t tmp = page2pa(p);
        pgdir[PDX(va)] = tmp | PTE_P | PTE_W ;

        return (KADDR(tmp) + PTX(va));

    }
    else 
    {
        uint32_t page_table = pgdir[PDX(va)] & ~0xfff;
        return (KADDR(page_table) + PTX(va));
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
    printf("free_mem---%x",free_mem);
    ppage_init();
    

    /*             kern page directory        */
    struct Page* p = ppage_alloc();
    p->ref_count++;
    kern_pgdir = KADDR((uint32_t*)page2pa(p));


    //  kern_pgdir : kernel addr. Its physics addr is PADDR(kern_pgdir)
    
    
    memset(kern_pgdir, 0, PGSIZE);
    

    // 创建内核空间到所有物理地址的直接映射
    int r = memory_map(kern_pgdir, 0xf0000000, 0, 0x10000000, PTE_W | PTE_P);
    if(r < 0)
        panic("fatal error: vm_init(): memory_map() error");



    
    printf("init done.kern_pgdir: %x", KADDR(kern_pgdir));
    printf("content: %x",kern_pgdir[PDX(0xf0000000)]);
    
    uint32_t p_kern_pgdir = (uint32_t)kern_pgdir - 0xf0000000;
    asm volatile("movl %0, %%cr3"
                : 
                :"r"(p_kern_pgdir));

 

    uint32_t cr0;
    asm volatile("movl %%cr0, %0"
                :"=r"(cr0)
                :);
    cr0 |= CR0_PE|CR0_PG|CR0_AM|CR0_WP|CR0_NE|CR0_MP;
    cr0 &= ~(CR0_TS|CR0_EM);
    asm volatile("movl %0, %%cr0"
                : 
                :"r"(cr0));
    

    vm_test();
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
    // 0x10000000 = 256MB, 256MB / 4KB = 64K = 64 * 1024
    ppage = init_alloc(sizeof(struct Page) * 64 * 1024);
    memset(ppage, 0, sizeof(struct Page) * 64 * 1024);
    
    uint32_t end = 64 * 1024, i = 0;

    // 0 ~ 640KB
    for(; i < 640 * 1024 / PGSIZE; i++)
    {
        ppage[i].ref_count = 0;
        ppage[i].next = &ppage[i + 1];
    }

    uint32_t tmp = i;

    // 640KB ~ 1MB
    for(; i < 1 * 1024 * 1024 / PGSIZE; i++)
    {
        ppage[i].ref_count = 1;
        ppage[i].next = NULL;
    }

    // 1MB ~ X, kern data
    for(; i < (free_mem & 0x0fffffff) / PGSIZE; i++)
    {
        ppage[i].ref_count = 1;
        ppage[i].next = NULL;
    }

    ppage[tmp].next = &ppage[i];

    // X ~ 256MB 
    for(; i < 0x10000000 / PGSIZE; i++)
    {
        ppage[i].ref_count = 0;
        ppage[i].next = &ppage[i + 1];
    }

    ppage[i].next = NULL;
    free_pages = &ppage[0];
    
}


void vm_test()
{
    // ppage_alloc  

    struct Page* p = ppage_alloc();
    p->ref_count++;
    printf("ppage : %x", page2pa(p));

    uint32_t* pte = page_entry(kern_pgdir, 0x200000);
    *pte = page2pa(p) | PTE_P | PTE_W;

    //printf("pte is %x", *pte);


    *(int*)0x200000 = 0x77777777;

    printf("test---%x", *(uint32_t*)0x200000);

    //printf("dd%x", kern_pgdir[PDX(0xf0100000)] & ~0xfff);

    printf("-------END--------");


}





