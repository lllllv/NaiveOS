#include "display.h"
#include "wrapper.h"
#include "vm.h"


unsigned
mc146818_read(unsigned reg)
{
	outb(0x70, reg);
	return inb(0x71);
}

static int
nvram_read(int r)
{
	return mc146818_read(r) | (mc146818_read(r + 1) << 8);
}

void kern()
{
	clear();

    uint32_t basemem, extmem, ext16mem, totalmem;

	// Use CMOS calls to measure available base & extended memory.
	// (CMOS calls return results in kilobytes.)
	basemem = nvram_read(0x15);
	extmem = nvram_read(0x17);
	ext16mem = nvram_read(0x30) * 64;

	// Calculate the number of physical pages available in both base
	// and extended memory.
	if (ext16mem)
		totalmem = 16 * 1024 + ext16mem;
	else if (extmem)
		totalmem = 1 * 1024 + extmem;
	else
		totalmem = basemem;




    printf("total is %d, base is %d, ext is %d, ext16 is %d",totalmem, basemem, extmem, ext16mem);
    vm_init();
    printf("vm init done");

    spin: 
        goto spin;
}



