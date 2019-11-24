#include "pci.h"

// found here:
// http://anadoxin.org/blog/pci-device-enumeration-using-ports-0xcf8-0xcfc.html

const u32int PCI_ENABLE_BIT = 0x80000000;
const u32int PCI_CONFIG_ADDRESS = 0xCF8;
const u32int PCI_CONFIG_DATA = 0xCFC;

// func - 0-7
// slot - 0-31
// bus - 0-255
u32int r_pci_32(u8int bus, u8int device, u8int func, u8int pcireg) {

  outl(PCI_ENABLE_BIT | (bus << 16) | (device << 11) | (func << 8) |
           (pcireg << 2),
       PCI_CONFIG_ADDRESS);

  return inl(PCI_CONFIG_DATA);
}

int init_pcilist(void) {

  u8int bus, device, func;
  u32int data;

  for (bus = 0; bus != 0xff; bus++) {
    for (device = 0; device < 32; device++) {
      for (func = 0; func < 8; func++) {

        data = r_pci_32(bus, device, func, 0);

        if (data != 0xffffffff) {
          //   k_printf(KERN_INFO "bus %d, device %d, func %d: vendor=0x%08x\n",
          //   bus,
          //            device, func, data);

          k_printf("device found! bus: %d", bus);
          k_printf(" device: %d", device);
          k_printf(" func: %d", func);
          k_printf(" data: 0x%x\n", data);

          u16int device = (data >> 16);
          u16int vendor = data & 0xFFFF;

          k_printf("vendor: 0x%x", vendor);
          k_printf(" device: 0x%x\n", device);
        }
      }
    }
  }

  return 0;
}

// https://wiki.osdev.org/PCI
u16int pciConfigReadWord(u8int bus, u8int slot, u8int func, u8int offset) {

  uint32_t address;
  uint32_t lbus = (uint32_t)bus;
  uint32_t lslot = (uint32_t)slot;
  uint32_t lfunc = (uint32_t)func;
  u16int tmp = 0;

  /* create configuration address as per Figure 1 */
  address = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) |
                       (offset & 0xfc) | ((uint32_t)0x80000000));

  /* write out the address */
  outl(0xCF8, address);

  /* read in the data */
  /* (offset & 2) * 8) = 0 will choose the first word of the 32 bits register */
  tmp = (u16int)((inl(0xCFC) >> ((offset & 2) * 8)) & 0xffff);

  return (tmp);
}

// https://wiki.osdev.org/PCI
u16int pciCheckVendor(u8int bus, u8int slot) {

  u16int vendor, device;

  // try and read the first configuration register. Since there are no
  // vendors that == 0xFFFF, it must be a non-existent device.
  if ((vendor = pciConfigReadWord(bus, slot, 0, 0)) != 0xFFFF) {
    device = pciConfigReadWord(bus, slot, 0, 2);
  }
  return (vendor);
}