ifneq ($(KERNELRELEASE),)
	obj-m := manager.o
else
	#KERNELDIR := $(EC535)/bbb/stock/stock-linux-4.19.82-ti-rt-r33
	KERNELDIR := /lib/modules/$(shell uname -r)/build
	PWD := $(shell pwd)
	ARCH := arm
	CROSS := arm-linux-gnueabihf-
	CFLAGS := "-std=c11 -Wall -Werror"

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS) CFLAGS=$(CFLAGS) modules

raspi:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) CFLAGS=$(CFLAGS) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) ARCH=$(ARCH) clean

endif
