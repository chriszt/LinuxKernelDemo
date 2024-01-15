#!/bin/sh

qemu-system-x86_64 -kernel /home/yl/Kernel/linux-hwe-5.15-5.15.0/arch/x86_64/boot/bzImage -initrd /home/yl/BusyBox/initrd-busybox.img -smp 2 -append "console=ttyS0 nokaslr" -enable-kvm -net nic,macaddr=52:54:00:12:34:56,model=e1000 -net bridge,id=net0,helper=/usr/lib/qemu/qemu-bridge-helper,br=virbr0 -nographic -S -s
