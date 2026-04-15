# Directories
SRC_DIR := kernel
TMP_DIR := tmp

# Tools
CC := gcc
AS := nasm
LD := ld
OBJCOPY := objcopy

# Flags
CFLAGS := -m32 -ffreestanding -fno-jump-tables -fno-pic -fno-pie -no-pie -fno-stack-protector \
					-fno-leading-underscore -nostdlib -nostartfiles -nodefaultlibs -I$(SRC_DIR)
ASFLAGS := -f elf32
LDFLAGS := -m elf_i386 -T linker.ld

# Files (ADD NEW FILES HERE 👇)
C_SOURCES := io.c vga.c kernel.c ata.c functions.c idt.c timer.c rtc.c
ASM_SOURCES := kernel_entry.asm interrupts.asm

# Auto-generated object lists
C_OBJECTS := $(addprefix $(TMP_DIR)/, $(C_SOURCES:.c=.o))
ASM_OBJECTS := $(addprefix $(TMP_DIR)/, $(ASM_SOURCES:.asm=.o))

OBJECTS := $(ASM_OBJECTS) $(C_OBJECTS)

# Outputs
KERNEL_TMP := $(TMP_DIR)/kernel.tmp
KERNEL_BIN := kernel.bin
OS_IMAGE := os-image.bin
DISK_IMG := disk.img

# Default target
all: run

# Ensure tmp directory exists
$(TMP_DIR):
	mkdir -p $(TMP_DIR)

# Compile C files
$(TMP_DIR)/%.o: $(SRC_DIR)/%.c | $(TMP_DIR)
	@echo "[C] $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Assemble ASM files
$(TMP_DIR)/%.o: $(SRC_DIR)/%.asm | $(TMP_DIR)
	@echo "[ASM] $<"
	$(AS) $< $(ASFLAGS) -o $@

# Link kernel
$(KERNEL_TMP): $(OBJECTS)
	@echo "[LD]"
	$(LD) $(LDFLAGS) $^ -o $@

# Convert to binary
$(KERNEL_BIN): $(KERNEL_TMP)
	@echo "[OBJCOPY]"
	$(OBJCOPY) -O binary $< $@

# Build OS image
$(OS_IMAGE): $(KERNEL_BIN) boot.asm
	@echo "[BOOT]"
	$(AS) boot.asm -f bin -o boot.bin
	cat boot.bin $(KERNEL_BIN) > $(OS_IMAGE)

# Only create disk.img if it doesn't exist
$(DISK_IMG):
	@echo "[CREATE DISK]"
	qemu-img create -f raw $(DISK_IMG) 4g

# Run in QEMU
run: $(OS_IMAGE) $(DISK_IMG)
	@echo "[RUN]"
	qemu-system-i386 \
		-drive format=raw,file=$(OS_IMAGE),index=0,if=floppy \
		-drive format=raw,file=disk.img,index=0,if=ide \
		-m 512 -vga std

# Clean build artifacts
clean:
	rm -rf $(TMP_DIR) *.bin *.img boot.bin

.PHONY: all run clean
