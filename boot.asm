.globl _start
_start:
.code16
    cli   /*  加了这个 莫名其妙的triple fault消失了 */
    cld   

/*   enable A20    */
seta20.1:
    inb     $0x64, %al
    testb   $0x2, %al
    jnz     seta20.1

    movb    $0xd1, %al
    outb    %al, $0x64

seta20.2:
    inb     $0x64, %al
    testb   $0x2, %al
    jnz     seta20.2

    movb    $0xdf, %al
    outb    %al, $0x60   

/*   entering protected mode    */
    lgdt gdt_desc
    movl    %cr0, %eax
    orl     $0x1, %eax 
    movl    %eax, %cr0 
    ljmp $0x8, $protected_mode

.code32
protected_mode:
    mov $0x10, %ax
    mov %ax, %ss 
    mov %ax, %ds 
    mov %ax, %es 
    mov %ax, %fs 
    mov %ax, %gs 
    xorl %ebp, %ebp 
    movl $0xA0000, %eax
    movl %eax, %esp
    call kernel_loader
spin:
    jmp spin

gdt:
    .long 0x00000000
    .long 0x00000000
code_seg:
    .long 0x0000FFFF
    .long 0x00CF9A00
data_seg:
    .long 0x0000FFFF
    .long 0x00CF9200

gdt_desc:
    .word 0x17
    .long gdt 



