obj-m := hello_dev.o 

#KDIR  ?= /lib/modules/$(shell uname -r)/build
KDIR  ?=  /srv/nfs/debian/lib/modules/5.0.0-rc7TRN0038+/build
PWD   := $(shell pwd)

all: module test

module:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

reinsert:
	rmmod hello_dev || true
	insmod ./hello_dev.ko

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f test
