
obj-m += vid_isp_ar0234.o

# EXTRA_CFLAGS += -DDEBUG

KERNEL_SRC ?= /usr/src/kernel

default:
	make -C ${KERNEL_SRC} M=$(CURDIR) modules

modules_install:
	make -C ${KERNEL_SRC} M=$(CURDIR) modules_install

clean:
	make -C ${KERNEL_SRC} M=$(CURDIR) clean
