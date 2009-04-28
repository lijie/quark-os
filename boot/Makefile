
.PHONY: all

all: loader.bin

loader.bin: loader.elf
	objcopy -S -O binary loader.elf loader.bin

loader.elf: loader.o
	ld loader.o -o loader.elf -Tboot.lds

loader.o: loader.S
	gcc $(CFLAGS) -c loader.S

