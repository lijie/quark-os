export ARCH		:= i386
export ROOT_PATH	:= $(shell pwd)
export BUILD_PATH	:= $(ROOT_PATH)/build
export KERNEL_PATH	:= $(ROOT_PATH)/kernel
export INC_PATH		:= $(ROOT_PATH)/include
export CFLAGS 		:= -O2 -fno-builtin -Wall -I$(INC_PATH)

.PHONY: all loader kernel

all: boot.img
#	cp boot.img ../../Bochs-2.3.6/boot.img

boot.img: loader kernel
	tools/mkimage boot/loader.bin kernel/kernel.bin

loader:
	make -C boot

kernel:
	make -C kernel

clean:
	find ./ -name "*.o" | xargs rm -rf
	find ./ -name "*.bin" | xargs rm -rf
	find ./ -name "*.o.asm" | xargs rm -rf
	rm -rf boot.img
