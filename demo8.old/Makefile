obj-m := demo.o

BASEINCLUDE ?= /home/yl/Kernel/linux-hwe-5.15-5.15.0
EXTRA_CFLAGS = -O1 -g

all:
	make -C $(BASEINCLUDE) M=$(PWD) modules

clean:
	make -C $(BASEINCLUDE) M=$(PWD) clean
