
obj-m += dummy_csi_sensor.o

# EXTRA_CFLAGS += -DDEBUG

KERNEL_VERSION ?= $(shell uname -r)
KERNEL_SRC ?= /lib/modules/$(KERNEL_VERSION)/build

dummy_csi_sensor.ko:
	make -C ${KERNEL_SRC} M=$(CURDIR) modules

modules_install: dummy_csi_sensor.ko
	make KERNELRELEASE=${KERNEL_VERSION} -C ${KERNEL_SRC} M=$(CURDIR) modules_install

clean:
	make -C ${KERNEL_SRC} M=$(CURDIR) clean

all: modules_install
	rmmod -f dummy_csi_sensor && modprobe dummy_csi_sensor
