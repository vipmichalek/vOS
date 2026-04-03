@echo off
if not exist "tmp" mkdir tmp

echo [1/5] assembler
nasm kernel\kernel_entry.asm -f elf32 -o tmp\kernel_entry.o
nasm kernel\interrupts.asm -f elf32 -o tmp\interrupts.o

echo [2/5] c
gcc -m32 -ffreestanding -fno-leading-underscore -Ikernel -c kernel\io.c -o tmp\io.o
gcc -m32 -ffreestanding -fno-leading-underscore -Ikernel -c kernel\vga.c -o tmp\vga.o
gcc -m32 -ffreestanding -fno-leading-underscore -Ikernel -c kernel\idt.c -o tmp\idt.o
gcc -m32 -ffreestanding -fno-leading-underscore -Ikernel -c kernel\ata.c -o tmp\ata.o
gcc -m32 -ffreestanding -fno-leading-underscore -Ikernel -c kernel\timer.c -o tmp\timer.o
gcc -m32 -ffreestanding -fno-leading-underscore -Ikernel -c kernel\functions.c -o tmp\functions.o
gcc -m32 -ffreestanding -fno-leading-underscore -Ikernel -c kernel\kernel.c -o tmp\kernel.o -fno-jump-tables

echo [3/5] link
ld -m i386pe -Ttext 0x1000 tmp\kernel_entry.o tmp\interrupts.o tmp\kernel.o tmp\vga.o tmp\io.o tmp\idt.o tmp\functions.o tmp\ata.o tmp\timer.o -o tmp\kernel.tmp

echo [4/5] binarka
objcopy -O binary tmp\kernel.tmp kernel.bin

echo [5/5] finalny
nasm boot.asm -f bin -o boot.bin
copy /b boot.bin + kernel.bin os-image.bin

echo koniec
@REM "C:\Users\micha\Downloads\qemu\qemu-img.exe" create -f raw D:\vos\hdd.img 4g
"C:\Users\micha\Downloads\qemu\qemu-system-i386.exe" -drive format=raw,file=D:\vos\os-image.bin,index=0,if=floppy -drive format=raw,file=D:\vos\hdd.img,index=0,if=ide -m 512 -vga std