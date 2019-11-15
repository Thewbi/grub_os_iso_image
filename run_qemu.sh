#!/bin/bash

#qemu-system-i386 -cdrom image.iso
#qemu-system-i386 -net nic,model=rtl8139 -no-kvm-irqchip -cdrom image.iso
qemu-system-i386 -net nic,model=rtl8139 -cdrom image.iso
