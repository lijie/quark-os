SOURCES  	= $(wildcard *.c)
ASM_SOURCES  	= $(wildcard *.S)

OBJS		:= $(SOURCES:.c=.o)
OBJS		+= $(ASM_SOURCES:.S=.o)
DEPS 		:= $(OBJS:.o=.d)

.PHONY: all

all: drivers.o

drivers.o: $(SOURCES)
	$(CC) $(CFLAGS) -c $(SOURCES) -o drivers.o

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
