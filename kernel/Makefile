SOURCES  	= $(wildcard *.c)
ASM_SOURCES  	= $(wildcard *.S)

OBJS		:= $(SOURCES:.c=.o)
OBJS		+= $(ASM_SOURCES:.S=.o)
DEPS 		:= $(OBJS:.o=.d)

.PHONY: all

all: kernel.bin

kernel.bin: kernel.elf
	$(OBJCOPY) -S -O binary kernel.elf kernel.bin

kernel.elf: $(OBJS)
#	ld  $(OBJS) -o kernel.elf -Map System.map -e startup -Tkernel.lds
	$(LD)  $(OBJS) -o kernel.elf -e startup -Tkernel.lds
	$(OBJDUMP) -t kernel.elf > System.map

%.o: %.S
	$(CC) $(CFLAGS) -c $<
	$(OBJDUMP) -D $@ > $@.asm

%.o: %.c
	$(CC) $(CFLAGS) -c $<
	$(OBJDUMP) -D $@ > $@.asm

%.d: %.c
	$(CC) -M $(CFLAGS) $< > $@ 

%.d: %.S
	$(CC) -M $(CFLAGS) $< > $@ 

ifneq ($(MAKECMDGOALS), clean)
include $(DEPS)
endif
