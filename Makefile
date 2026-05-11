CC = gcc
CFLAGS = -m32 -ffreestanding -fno-pic -fno-stack-protector -fno-asynchronous-unwind-tables -nostdlib -I.
ASM = nasm
LD = ld

# The objects that make up the "Nucleus"
OBJS = kernel.o interrupt.o atom.o

all: uran_img.bin

# Compile the Heart
kernel.o: kernel.c
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o

# Compile the Task (ATOM)
atom.o: atom.c
	$(CC) $(CFLAGS) -c atom.c -o atom.o

interrupt.o: interrupt.s
	$(ASM) -f elf32 interrupt.s -o interrupt.o

# Link all objects together into the final kernel binary
kernel.bin: $(OBJS)
	$(LD) -m elf_i386 -T linker.ld $(OBJS) -o kernel.bin --oformat binary

boot.bin: boot.s
	$(ASM) -f bin boot.s -o boot.bin

uran_img.bin: boot.bin kernel.bin
	cat boot.bin kernel.bin > uran_img.bin
	truncate -s 10240 uran_img.bin

run: all
	qemu-system-i386 -drive format=raw,file=uran_img.bin

clean:
	rm -f *.o *.bin uran_img.bin
