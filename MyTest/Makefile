BASEINCLUDE ?= /home/yl/proj/linux-hwe-5.15-5.15.0

mytest-objs := my_test.o
obj-m := mytest.o

all :
	make -C $(BASEINCLUDE) M=$(PWD) modules

clean :
	rm -rf *.ko
