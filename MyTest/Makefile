BASEINCLUDE ?= /home/yl/Kernel/linux-hwe-5.15-5.15.0

obj-m := MyTest.o

all:
	make -C $(BASEINCLUDE) M=$(PWD) modules

clean:
	make -C $(BASEINCLUDE) M=$(PWD) clean
