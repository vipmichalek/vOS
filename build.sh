mkdir -p tmp

echo [1/5] assembler
nasm kernel/kernel_entry.asm -f elf32 -o tmp/kernel_entry.o
nasm kernel/interrupts.asm -f elf32 -o tmp/interrupts.o

echo [2/5] c
gcc -m32 -ffreestanding -fno-jump-tables -fno-pic -fno-pie -no-pie -fno-stack-protector -Ikernel -c kernel/io.c -o tmp/io.o
gcc -m32 -ffreestanding -fno-jump-tables -fno-pic -fno-pie -no-pie -fno-stack-protector -Ikernel -c kernel/vga.c -o tmp/vga.o
gcc -m32 -ffreestanding -fno-jump-tables -fno-pic -fno-pie -no-pie -fno-stack-protector -Ikernel -c kernel/kernel.c -o tmp/kernel.o -fno-jump-tables

echo [3/5] link
ld -m elf_i386 -Ttext 0x1000 tmp/kernel_entry.o tmp/interrupts.o tmp/kernel.o tmp/vga.o tmp/io.o -o tmp/kernel.tmp

echo [4/5] binary
objcopy -O binary tmp/kernel.tmp kernel.bin

echo [5/5] finilizing
nasm boot.asm -f bin -o boot.bin
cat boot.bin kernel.bin >os-image.bin
qemu-img create -f raw disk.img 4g
qemu-system-i386 -drive format=raw,file=os-image.bin,index=0,if=floppy -drive format=raw,file=disk.img,index=0,if=ide -m 512 -vga std
