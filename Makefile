CC = gcc
AS = nasm
LD = ld
CFLAGS = -m64 -ffreestanding -O2 -Wall -Wextra -fno-stack-protector -fno-pie -mcmodel=small -mno-red-zone -fno-asynchronous-unwind-tables
LDFLAGS = -m elf_x86_64 -T linker.ld -z noexecstack

KERNEL_OBJS = kernel.o atom.o interrupt.o app_space.o run.o

all: uran_img.bin

uran_img.bin: boot.bin kernel.bin
	rm -f uran_img.bin
	cat boot.bin kernel.bin > uran_img.bin
	truncate -s +50K uran_img.bin

boot.bin: boot.s
	$(AS) -f bin boot.s -o boot.bin

kernel.bin: $(KERNEL_OBJS)
	$(LD) $(LDFLAGS) -o kernel.elf $(KERNEL_OBJS)
	objcopy -O binary kernel.elf kernel.bin

interrupt.o: interrupt.s
	$(AS) -f elf64 interrupt.s -o interrupt.o

kernel.o: kernel.c
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o

atom.o: atom.c
	$(CC) $(CFLAGS) -c atom.c -o atom.o

app_space.o: app_space.c
	$(CC) $(CFLAGS) -c app_space.c -o app_space.o

run.o: run.c
	$(CC) $(CFLAGS) -c run.c -o run.o

run: all
	qemu-system-x86_64 -drive format=raw,file=uran_img.bin

clean:
	rm -f *.bin *.o *.elf uran_img.bin