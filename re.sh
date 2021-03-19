
# bootloader part:
x86_64-elf-gcc -Os -nostdinc -fno-builtin -fno-omit-frame-pointer -std=gnu99 -static -I . -Wall -Wno-format -Wno-unused -Werror -gstabs -m32 -fno-tree-ch -c -o kernel_loader.o kernel_loader.c &&
i386-elf-as   boot.asm -o boot.o &&
i386-elf-ld -m elf_i386 -N  -Ttext 0x7C00  boot.o kernel_loader.o  -o boot.out &&
i386-elf-objcopy -O binary -j .text boot.out boot &&
perl sign.pl boot &&


# kernel part:
x86_64-elf-gcc -Os -nostdinc -fno-builtin -fno-omit-frame-pointer -std=gnu99 -static -I . -Wall -Wno-format -Wno-unused -Werror -gstabs -m32 -fno-tree-ch -c temp_pgdir.c -o temp_pgdir.o &&
x86_64-elf-gcc -Os -nostdinc -fno-builtin -fno-omit-frame-pointer -std=gnu99 -static -I . -Wall -Wno-format -Wno-unused -Werror -gstabs -m32 -fno-tree-ch -c kern.c -o kern.o &&
i386-elf-as kern_init.asm -o kern_init.o && 
i386-elf-ld -m elf_i386 -N -T kernel.ld -nostdlib kern_init.o temp_pgdir.o kern.o -o kern_init.out



# make kernel image file
dd if=/dev/zero of=kern.img count=200 &&
dd if=boot of=kern.img conv=notrunc &&
dd if=kern_init.out of=kern.img seek=1 conv=notrunc &&


# run qemu 
qemu-system-i386 -drive file=kern.img,index=0,media=disk,format=raw -serial mon:stdio  -gdb tcp::1234 

