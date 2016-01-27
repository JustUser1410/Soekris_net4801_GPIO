obj-m += Soekris_GPIO.o
KERNELDIR = ~/buildroot/output/build/linux-3.10.85
TOOLCHAIN = ~/buildroot/output/host/usr/bin/i586-linux-

all:
	make ARCH=i386 CROSS_COMPILE=$(TOOLCHAIN) -C $(KERNELDIR) M=$(PWD) modules

clean:
	make −C $(KERNELDIR) M=$(PWD) clean
