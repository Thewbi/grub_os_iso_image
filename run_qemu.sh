#!/bin/bash

#qemu-system-i386 -cdrom image.iso
#qemu-system-i386 -net nic,model=rtl8139 -no-kvm-irqchip -cdrom image.iso

#qemu-system-i386 -net nic,model=rtl8139 -cdrom image.iso

#qemu-system-i386 -net nic,model=rtl8139,mac=00:00:00:11:11:11 -cdrom image.iso
#qemu-system-i386 -net nic,model=rtl8139,macaddr=00:00:00:11:11:11 -cdrom image.iso

#qemu-system-i386 -netdev tap,ifname=tap0,id=mynet0,script=no -device rtl8139,netdev=mynet0,mac=00:00:00:11:11:11 -cdrom image.iso

# Description for network devices:
# https://github.com/qemu/qemu/blob/master/docs/qdev-device-use.txt

# working
# splitting over several lines only works if there are no trailing spaces after the backslash!
#qemu-system-i386 \
#-monitor stdio \
#-cdrom image.iso \
#-netdev user,id=network0 \
#-device rtl8139,netdev=network0,mac=52:54:00:12:34:56 \
#-object filter-dump,id=network_filter_object,netdev=network0,file=dump.dat

/home/wbi/dev/qemu/build/i386-softmmu/qemu-system-i386 \
-monitor stdio \
-cdrom image.iso \
-netdev user,id=network0 \
-device rtl8139,netdev=network0,mac=52:54:00:12:34:56 \
-object filter-dump,id=network_filter_object,netdev=network0,file=dump.dat

# Damn Small Linux - http://distro.ibiblio.org/damnsmall/release_candidate/
#qemu-system-i386 \
#-monitor stdio \
#-cdrom ~/Downloads/os_images/dsl-4.11.rc2.iso \
#-netdev user,id=network0 \
#-device rtl8139,netdev=network0,mac=52:54:00:12:34:56 \
#-object filter-dump,id=network_filter_object,netdev=network0,file=dump.dat

# slax-32bit-9.11.0
#qemu-system-i386 \
#-monitor stdio \
#-cdrom ~/Downloads/os_images/slax-32bit-9.11.0.iso \
#-netdev user,id=network0 \
#-device rtl8139,netdev=network0,mac=52:54:00:12:34:56 \
#-object filter-dump,id=network_filter_object,netdev=network0,file=dump.dat

# tap networking
# https://gist.github.com/extremecoders-re/e8fd8a67a515fee0c873dcafc81d811c
#qemu-system-i386 \
#-monitor stdio \
#-cdrom image.iso \
#-netdev tap,id=mynet0,ifname=tap0,script=no,downscript=no \
#-device rtl8139,netdev=mynet0,mac=52:54:00:12:34:56

#qemu-system-i386 \
#-monitor stdio \
#-cdrom image.iso \
#-netdev tap,id=mynet0,ifname=tap0 \
#-device rtl8139,netdev=mynet0,mac=52:54:00:12:34:56