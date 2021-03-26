#include "types.h"


//#define PADDR(x)  ((x))
#define PDX(x)      ((x) >> 22)
#define PTX(x)      (((x) >> 12) & 0x3ff)
#define PGSIZE      4096


struct Page 
{
    uint32_t ref_count;
    struct Page* next;
    uint32_t flags;
};


void vm_init();
void* init_alloc(uint32_t size);
void ppage_init();
uint32_t page2pa(struct Page* p);
struct Page* pa2page(uint32_t);
