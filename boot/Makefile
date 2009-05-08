
.PHONY: all

all: loader.bin

loader.bin: loader.elf
	$(OBJCOPY) -S -O binary loader.elf loader.bin

loader.elf: loader.o
	$(LD) loader.o -o loader.elf -Tboot.lds

loader.o: loader.S
	$(CC) $(CFLAGS) -c loader.S

