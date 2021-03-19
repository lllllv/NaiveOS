

.section .text

.globl _start
_start = entry - 0xf0000000


entry: 
    cli
    cld
    movl $(temp_pgdir - 0xf0000000), %eax 
    movl %eax, %cr3 
    movl %cr0, %eax 
    orl $0x80010001, %eax
    movl %eax, %cr0 
    movl $relocated, %eax 
    jmp *%eax 

relocated:
    movl $0, %eax
    movl %eax, %ebp 
    movl $bootstack, %eax 
    movl %eax, %esp
    call kern

spin:
    jmp spin 


.section .data 
    .p2align 12
bootstack:
    .space 4096

