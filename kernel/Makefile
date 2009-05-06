SOURCES  	= $(wildcard *.c)
ASM_SOURCES  	= $(wildcard *.S)

OBJS		:= $(SOURCES:.c=.o)
OBJS		+= $(ASM_SOURCES:.S=.o)

.PHONY: all

all: kernel.bin

kernel.bin: kernel.elf
	objcopy -S -O binary kernel.elf kernel.bin

kernel.elf: $(OBJS)
	ld  $(OBJS) -o kernel.elf -Map System.map -e startup -Tkernel.lds

%.o: %.S
	gcc $(CFLAGS) -c $<
	objdump -D $@ > $@.asm

%.o: %.c
	gcc $(CFLAGS) -c $<
	objdump -D $@ > $@.asm

