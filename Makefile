CC = gcc
AS = nasm
LD = ld
CFLAGS = -m64 -ffreestanding -O2 -Wall -Wextra -fno-stack-protector -fno-pie -mcmodel=kernel -mno-red-zone
LDFLAGS = -m elf_x86_64 -T linker.ld --oformat binary

all: uran_img.bin

uran_img.bin: boot.bin kernel.bin
	cat boot.bin kernel.bin > uran_img.bin

boot.bin: boot.s
	$(AS) -f bin boot.s -o boot.bin

kernel.bin: kernel.o atom.o interrupt.o
	$(LD) $(LDFLAGS) -o kernel.bin kernel.o atom.o interrupt.o

interrupt.o: interrupt.s
	$(AS) -f elf64 interrupt.s -o interrupt.o

kernel.o: kernel.c
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o

atom.o: atom.c
	$(CC) $(CFLAGS) -c atom.c -o atom.o

run: all
	qemu-system-x86_64 -drive format=raw,file=uran_img.bin

clean:
	rm -f *.bin *.o uran_img.bin