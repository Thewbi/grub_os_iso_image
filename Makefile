CP := cp
RM := rm -rf
MKDIR := mkdir -pv

BIN = kernel
CFG = grub.cfg
ISO_PATH := iso
BOOT_PATH := $(ISO_PATH)/boot
GRUB_PATH := $(BOOT_PATH)/grub

#GCC := gcc
GCC := ~/dev/cross/install/bin/i386-elf-gcc

#LD := ld
LD := ~/dev/cross/install/bin/i386-elf-ld

.PHONY: all

all: bootloader kernel linker modules iso
	@echo Make has completed.

bootloader: boot.asm
	nasm -f elf32 boot.asm -o boot.o
	nasm -f elf32 interrupt.asm -o interrupt.o
	nasm -f elf32 gdt.asm -o gdt.o

kernel: kernel.c placement_memory.c isr.c descriptor_tables.c common.c memory_map.c stack.c bitmap.c
	$(GCC) -m32 -std=c99 -ffreestanding -nostdinc -c kernel.c -o kernel.o
	$(GCC) -m32 -std=c99 -ffreestanding -nostdinc -c placement_memory.c -o placement_memory.o
	$(GCC) -m32 -std=c99 -ffreestanding -nostdinc -c isr.c -o isr.o
	$(GCC) -m32 -std=c99 -ffreestanding -nostdinc -c descriptor_tables.c -o descriptor_tables.o
	$(GCC) -m32 -std=c99 -ffreestanding -nostdinc -c common.c -o common.o
	$(GCC) -m32 -std=c99 -ffreestanding -nostdinc -c memory_map.c -o memory_map.o
	$(GCC) -m32 -std=c99 -ffreestanding -nostdinc -c stack.c -o stack.o
	$(GCC) -m32 -std=c99 -ffreestanding -nostdinc -c bitmap.c -o bitmap.o
	$(GCC) -m32 -std=c99 -ffreestanding -nostdinc -c paging.c -o paging.o
	$(GCC) -m32 -std=c99 -ffreestanding -nostdinc -c pci.c -o pci.o
	$(GCC) -m32 -std=c99 -ffreestanding -nostdinc -c multiboot.c -o multiboot.o
	$(GCC) -m32 -std=c99 -ffreestanding -nostdinc -c heap_buddy_system_evanw.c -o heap_buddy_system_evanw.o

linker: linker.ld boot.o kernel.o placement_memory.o isr.o gdt.o interrupt.o descriptor_tables.o common.o memory_map.o stack.o bitmap.o paging.o pci.o multiboot.o
	$(LD) -m elf_i386 -T linker.ld -o kernel \
	boot.o \
	kernel.o \
	placement_memory.o \
	isr.o \
	gdt.o \
	interrupt.o \
	descriptor_tables.o \
	common.o \
	memory_map.o \
	stack.o \
	bitmap.o \
	paging.o \
	multiboot.o \
	heap_buddy_system_evanw.o \
	pci.o

iso: kernel
	$(MKDIR) $(GRUB_PATH)
	$(CP) $(BIN) $(BOOT_PATH)
	$(CP) $(CFG) $(GRUB_PATH)
	grub-file --is-x86-multiboot $(BOOT_PATH)/$(BIN)
	grub-mkrescue -o image.iso $(ISO_PATH)

modules:
	$(MKDIR) $(GRUB_PATH)
	$(CP) test $(BOOT_PATH)

.PHONY: clean
clean:
	$(RM) *.o $(BIN) *iso image.img
