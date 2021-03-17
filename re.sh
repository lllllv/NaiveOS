x86_64-elf-gcc -Os -nostdinc -fno-builtin -fno-omit-frame-pointer -std=gnu99 -static -I . -Wall -Wno-format -Wno-unused -Werror -gstabs -m32 -fno-tree-ch -c -o kernel_loader.o kernel_loader.c &&
#x86_64-elf-gcc -O1 -nostdinc -fno-builtin -fno-omit-frame-pointer -std=gnu99 -static -I . -Wall -Wno-format -Wno-unused -Werror -gstabs -m32 -fno-tree-ch -c -o lib.o lib.c &&
i386-elf-as   boot1.asm -o boot1.o &&
i386-elf-ld -m elf_i386 -N  -Ttext 0x7C00  boot1.o kernel_loader.o  -o boot1.out &&
i386-elf-objcopy -O binary -j .text boot1.out boot1 &&
perl sign.pl boot1 &&
#dd if=/dev/random of=kern.img count=2 &&
dd if=boot1 of=kern.img conv=notrunc &&

#qemu-system-i386 boot1

qemu-system-i386 -drive file=kern.img,index=0,media=disk,format=raw -serial mon:stdio  -gdb tcp::1234  

#i386-elf-as  boot1.asm -o boot1.o
#i386-elf-objcopy -O binary -j .text boot1.o boot1
#perl sign.pl boot1
#qemu-system-i386 boot1
