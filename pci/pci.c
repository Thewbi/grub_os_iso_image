#include "pci.h"

// found here:
// http://anadoxin.org/blog/pci-device-enumeration-using-ports-0xcf8-0xcfc.html
// The ports are also described here: https://wiki.osdev.org/PCI

const u32int PCI_ENABLE_BIT = 0x80000000;
const u32int PCI_CONFIG_ADDRESS = 0xCF8;
const u32int PCI_CONFIG_DATA = 0xCFC;

// func - 0-7
// slot - 0-31
// bus - 0-255
//
// described here: https://en.wikipedia.org/wiki/PCI_configuration_space under
// the section "software implementation"
// parameter pcireg: 0 will read the first 32bit dword of the pci control space
// which is DeviceID and Vendor ID
// pcireg = 1 will read the second 32bit dword which is status and command
// and so on...
u32int r_pci_32(u8int bus, u8int device, u8int func, u8int pcireg) {

  // compute the index
  //
  // pcireg is left shifted twice to multiply it by 4 because each register
  // is 4 byte long (32 bit registers)
  u32int index = PCI_ENABLE_BIT | (bus << 16) | (device << 11) | (func << 8) |
                 (pcireg << 2);

  // write the index value onto the index port
  outl(index, PCI_CONFIG_ADDRESS);

  // read a value from the data port
  return inl(PCI_CONFIG_DATA);
}

// https://wiki.osdev.org/PCI#Configuration_Mechanism_.231
//
// param: offset: 0 is the first word in the pci configuration space
uint16_t pciConfigReadWord(u8int bus, u8int slot, u8int func, u8int offset) {

  uint32_t address;
  uint32_t lbus = (uint32_t)bus;
  uint32_t lslot = (uint32_t)slot;
  uint32_t lfunc = (uint32_t)func;
  uint16_t tmp = 0;

  // create configuration address as per Figure 1
  address = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) |
                       (offset & 0xfc) | ((uint32_t)0x80000000));

  // write out the address
  outl(address, 0xCF8);

  // read in the data
  // (offset & 2) * 8) = 0 will choose the first word of the 32 bits register
  tmp = (uint16_t)((inl(0xCFC) >> ((offset & 2) * 8)) & 0xffff);

  return (tmp);
}

int pcibios_read_config_byte(unsigned int bus, unsigned int device,
                             unsigned int device_fn, unsigned int where) {

  u32int index = PCI_ENABLE_BIT | (bus << 16) | (device << 11) |
                 (device_fn << 8) | (where & ~3);

  outl(index, 0xCF8);

  return inb(0xCFC + (where & 3));
}

u32int w_pci_32(u8int bus, u8int device, u8int func, u8int pcireg,
                u32int value) {

  // compute the index
  //
  // pcireg is left shifted twice to multiply it by 4 because each register
  // is 4 byte long (32 bit registers)
  u32int index = PCI_ENABLE_BIT | (bus << 16) | (device << 11) | (func << 8) |
                 (pcireg << 2);
  // uint32_t index =
  //     (uint32_t)((bus << 16) || (device << 11) | (func << 8) | (pcireg &
  //     0xFC) |
  //                                   ((uint32_t)0x80000000));

  // write the index value onto the index port
  outl(index, PCI_CONFIG_ADDRESS);

  // read a value from the data port
  // return inl(PCI_CONFIG_DATA);

  outl(value, PCI_CONFIG_DATA);
}

u32int w_pci_8(u8int bus, u8int device, u8int func, u8int pcireg, u8int value) {

  // compute the index
  //
  // pcireg is left shifted twice to multiply it by 4 because each register
  // is 4 byte long (32 bit registers)
  u32int index = PCI_ENABLE_BIT | (bus << 16) | (device << 11) | (func << 8) |
                 (pcireg & ~3);

  // write the index value onto the index port
  outl(index, PCI_CONFIG_ADDRESS);
  // outl(index, PCI_CONFIG_ADDRESS + 1);
  // outl(index, PCI_CONFIG_ADDRESS + (pcireg & 3));

  // outb(index, PCI_CONFIG_ADDRESS);
  // outb(index, PCI_CONFIG_ADDRESS + (pcireg & ~3));
  // outb(index, PCI_CONFIG_ADDRESS + (pcireg & 3));

  // read a value from the data port
  // return inl(PCI_CONFIG_DATA);

  outb(value, PCI_CONFIG_DATA);
  // outb(value, PCI_CONFIG_DATA + (pcireg & 3));
  // outb(value, PCI_CONFIG_DATA + 1);
  // outb(value, PCI_CONFIG_DATA + (pcireg & ~3));
}

static void pcibios_write_config_dword(u8int bus, u8int device, u8int func,
                                       u8int pcireg, u32int value) {
  // compute the index
  //
  // pcireg is left shifted twice to multiply it by 4 because each register
  // is 4 byte long (32 bit registers)
  u32int index = PCI_ENABLE_BIT | (bus << 16) | (device << 11) | (func << 8) |
                 (pcireg << 2);

  // write the index value onto the index port
  outl(index, PCI_CONFIG_ADDRESS);

  // write a value to the data port
  // outb(value, PCI_CONFIG_DATA + (pcireg & 3));
  // outl(value, PCI_CONFIG_DATA + (pcireg));
  outl(value, PCI_CONFIG_DATA);
}

// static void pcibios_write_config_byte(u8int bus, u8int device, u8int func,
//                                       u8int pcireg, unsigned char value) {
//   // compute the index
//   // (Bus Device Function, BDF, addressing a device geographically)
//   // https://en.wikipedia.org/wiki/PCI_configuration_space
//   u32int index = PCI_ENABLE_BIT | (bus << 16) | (device << 11) | (func << 8)
//   |
//                  (pcireg << 2);

//   // write the index value onto the index port
//   outl(index, PCI_CONFIG_ADDRESS);

//   // write a value to the data port
//   // outb(value, PCI_CONFIG_DATA + (pcireg & 3));
//   outb(value, PCI_CONFIG_DATA);
// }

// https://github.com/coreboot/coreboot/blob/master/util/romcc/tests/raminit_test1.c
static unsigned int config_cmd(unsigned char bus, unsigned devfn,
                               unsigned where) {
  return 0x80000000 | (bus << 16) | (devfn << 8) | (where & ~3);
}

static void pcibios_write_config_byte(unsigned char bus, unsigned devfn,
                                      unsigned where, unsigned char value) {
  outl(config_cmd(bus, devfn, where), 0xCF8);
  outb(value, 0xCFC + (where & 3));
}

// Read from the configuration space.
// offset - tells the location to write to. Offset 0 is the start of the
// pci configuration header
//
// https:stackoverflow.com/questions/47858368/pci-configuration-space-registers-write-values
uint16_t pci_read_word(uint16_t bus, uint16_t slot, uint16_t func,
                       uint16_t offset) {

  // k_printf("pci_read_word()\n");

  uint64_t address;
  uint64_t lbus = (uint64_t)bus;
  uint64_t lslot = (uint64_t)slot;
  uint64_t lfunc = (uint64_t)func;
  uint16_t tmp = 0;

  // 0xfc = 1111 1100

  // address = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) |
  //                      (offset & 0xfc) | ((uint32_t)PCI_ENABLE_BIT));

  address = (uint64_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) |
                       (offset & 0xfc) | ((uint64_t)PCI_ENABLE_BIT));

  outl(address, PCI_CONFIG_ADDRESS);

  tmp = (uint16_t)((inl(PCI_CONFIG_DATA) >> ((offset & 2) * 8)) & 0xffff);

  return (tmp);

  // return inw(PCI_CONFIG_DATA);
}

// Offset 0 is the start of the pci header
uint16_t pci_write_word(uint16_t bus, uint16_t slot, uint16_t func,
                        uint16_t offset, uint16_t data) {
  uint64_t address;

  uint64_t lbus = (uint64_t)bus;
  uint64_t lslot = (uint64_t)slot;
  uint64_t lfunc = (uint64_t)func;

  uint32_t tmp = 0;

  address = (uint64_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) |
                       (offset & 0xfc) | ((uint32_t)PCI_ENABLE_BIT));

  // set the address to write to
  outl(address, PCI_CONFIG_ADDRESS);

  tmp = inl(PCI_CONFIG_DATA);

  // reset the word at the offset
  tmp &= ~(0xFFFF << ((offset & 0x2) * 8));

  // write the data at the offset
  tmp |= data << ((offset & 0x2) * 8);

  // set address again just to be sure
  outl(address, PCI_CONFIG_ADDRESS);

  // write data
  outl(tmp, PCI_CONFIG_DATA);

  // read back data;
  return pci_read_word(bus, slot, func, offset);
}

// Delay between EEPROM clock transitions.
// No extra delay is needed with 33Mhz PCI, but 66Mhz may change this.

#define eeprom_delay() inl(ee_addr)

// The EEPROM commands include the alway-set leading bit.
#define EE_WRITE_CMD (5 << 6)
#define EE_READ_CMD (6 << 6)
#define EE_ERASE_CMD (7 << 6)

static int read_eeprom(long ioaddr, int location) {

  unsigned retval = 0;
  long ee_addr = ioaddr + Cfg9346;
  int read_cmd = location | EE_READ_CMD;

  outb(EE_ENB & ~EE_CS, ee_addr);
  outb(EE_ENB, ee_addr);

  // Shift the read command bits out.
  for (int i = 10; i >= 0; i--) {

    int dataval = (read_cmd & (1 << i)) ? EE_DATA_WRITE : 0;

    outb(EE_ENB | dataval, ee_addr);
    eeprom_delay();

    outb(EE_ENB | dataval | EE_SHIFT_CLK, ee_addr);
    eeprom_delay();
  }

  outb(EE_ENB, ee_addr);
  eeprom_delay();

  for (int i = 16; i > 0; i--) {

    outb(EE_ENB | EE_SHIFT_CLK, ee_addr);
    eeprom_delay();

    retval = (retval << 1) | ((inb(ee_addr) & EE_DATA_READ) ? 1 : 0);

    outb(EE_ENB, ee_addr);
    eeprom_delay();
  }

  // Terminate the EEPROM access.
  outb(~EE_CS, ee_addr);

  return retval;
}

int init_pcilist(void) {

  u8int bus, device, func;
  u32int data;

  unsigned char pci_bus = 0;
  unsigned char pci_device = 0;
  unsigned char pci_device_fn = 0;
  u8int pci_latency = 0;

  int realtek8319Found = 0;

  // there are 256 busses allowed
  for (bus = 0; bus != 0xff; bus++) {

    // per bus there can be at most 32 devices
    for (device = 0; device < 32; device++) {

      // every device can be multi function device of up to 8 functions
      for (func = 0; func < 8; func++) {

        data = r_pci_32(bus, device, func, 0);
        if (data != 0xffffffff) {

          //   k_printf(KERN_INFO "bus %d, device %d, func %d: vendor=0x%08x\n",
          //   bus,
          //            device, func, data);

          // k_printf("device found! bus: %d", bus);
          // k_printf(" device: %d", device);
          // k_printf(" func: %d", func);
          // k_printf(" data: 0x%x\n", data);

          u16int device_value = (data >> 16);
          u16int vendor = data & 0xFFFF;

          // k_printf("vendor: 0x%x", vendor);
          // k_printf(" device: 0x%x\n", device_value);

          // 10ec, 8139

          realtek8319Found = 0;
          if (vendor == 0x10ec && device_value == 0x8139) {

            realtek8319Found = 1;

            pci_bus = bus;
            pci_device = device;
            pci_device_fn = func;

            k_printf("RTL8139 found! bus: %d", pci_bus);
            k_printf(" device: %d", pci_device);
            k_printf(" func: %d \n", pci_device_fn);
          }

          // realtek8319Found = 0;
          if (realtek8319Found) {

            // data = r_pci_32(bus, device, func, 1);
            // if (data != 0xffffffff) {

            //   u16int status = (data >> 16);
            //   u16int command = data & 0xFFFF;

            //   // 0x0000
            //   k_printf("status: 0x%x", status);

            //   // 0x0103 = 0000 0001 0000 0011 = 259d
            //   k_printf(" command: 0x%x\n", command);
            // }

            // for (int i = 0; i < 20; i += 2) {
            //   k_printf("Before %d: 0x%x\n", i,
            //            pciConfigReadWord(bus, device, func, i));
            //   pci_write_word(bus, device, func, i, 0xAAAA);
            //   k_printf("After %d: 0x%x\n", i,
            //            pciConfigReadWord(bus, device, func, i));
            // }

            // k_printf("Before %d: 0x%x\n", 0x0C,
            //          pciConfigReadWord(bus, device, func, 0x0C));
            // pci_write_word(bus, device, func, 0x0C, 0xAAAA);
            // k_printf("After  %d: 0x%x\n", 0x0C,
            //          pciConfigReadWord(bus, device, func, 0x0C));

            // k_printf("Before 0x%x: 0x%x\n", 0x3C,
            //          pciConfigReadWord(bus, device, func, 0x3C));
            // pci_write_word(bus, device, func, 0x3C, 0xAAAA);
            // k_printf("After  %x: 0x%x\n", 0x3C,
            //          pciConfigReadWord(bus, device, func, 0x3C));

            // k_printf("Before 0x%d: 0x%x\n", 0x3E,
            //          pciConfigReadWord(bus, device, func, 0x3E));
            // pci_write_word(bus, device, func, 0x3E, 0xAAAA);
            // k_printf("After  0x%d: 0x%x\n", 0x3E,
            //          pciConfigReadWord(bus, device, func, 0x3E));

            // k_printf("**************************************************\n");
            // for (int i = 0; i < 0x0F; i++) {
            //   k_printf("0x%x", pcibios_read_config_byte(bus, device, func,
            //                                             (i * 4) + 0x03));
            //   k_printf(" 0x%x", pcibios_read_config_byte(bus, device, func,
            //                                              (i * 4) + 0x02));
            //   k_printf(" 0x%x", pcibios_read_config_byte(bus, device, func,
            //                                              (i * 4) + 0x01));
            //   k_printf(" 0x%x\n", pcibios_read_config_byte(bus, device, func,
            //                                                (i * 4) + 0x00));
            // }
            // k_printf("-----------------------------------------------------\n");

            // for (int i = 0; i < 0x0F; i++) {

            //   pci_write_word(bus, device, func, (i * 4) + 0x00, 0xAAAA);
            //   pci_write_word(bus, device, func, (i * 4) + 0x02, 0xAAAA);
            // }

            // k_printf("**************************************************\n");
            // for (int i = 0; i < 0x0F; i++) {
            //   k_printf("0x%x", pcibios_read_config_byte(bus, device, func,
            //                                             (i * 4) + 0x03));
            //   k_printf(" 0x%x", pcibios_read_config_byte(bus, device, func,
            //                                              (i * 4) + 0x02));
            //   k_printf(" 0x%x", pcibios_read_config_byte(bus, device, func,
            //                                              (i * 4) + 0x01));
            //   k_printf(" 0x%x\n", pcibios_read_config_byte(bus, device, func,
            //                                                (i * 4) + 0x00));
            // }
            // k_printf("-----------------------------------------------------\n");

            // // vendor
            // k_printf("ReadByte: 0x%x\n",
            //          pcibios_read_config_byte(bus, device, func, 0x00));
            // k_printf("ReadByte: 0x%x\n",
            //          pcibios_read_config_byte(bus, device, func, 0x01));

            // // device
            // k_printf("ReadByte: 0x%x\n",
            //          pcibios_read_config_byte(bus, device, func, 0x02));
            // k_printf("ReadByte: 0x%x\n",
            //          pcibios_read_config_byte(bus, device, func, 0x03));

            // // command
            // k_printf("ReadByte: 0x%x\n",
            //          pcibios_read_config_byte(bus, device, func, 0x04));
            // k_printf("ReadByte: 0x%x\n",
            //          pcibios_read_config_byte(bus, device, func, 0x05));

            // // status
            // k_printf("ReadByte: 0x%x\n",
            //          pcibios_read_config_byte(bus, device, func, 0x06));
            // k_printf("ReadByte: 0x%x\n",
            //          pcibios_read_config_byte(bus, device, func, 0x07));

            // // Cache Line Size
            // k_printf("Cache Line Size: 0x%x\n",
            //          pcibios_read_config_byte(bus, device, func, 0x0C));
            // // Latency Timer
            // k_printf("Latency Timer: 0x%x\n",
            //          pcibios_read_config_byte(bus, device, func, 0x0D));
            // // Header Type
            // k_printf("Header Type: 0x%x\n",
            //          pcibios_read_config_byte(bus, device, func, 0x0E));
            // // BIST
            // k_printf("BIST: 0x%x\n",
            //          pcibios_read_config_byte(bus, device, func, 0x0F));

            // int register_index = 3;

            // k_printf("Test 1\n");
            // data = 0;
            // data = r_pci_32(bus, device, func, register_index);
            // k_printf("data 1: 0x%x\n", data);

            // // only writes the first byte!!!!
            // w_pci_32(bus, device, func, register_index, 0xFFFFFFFF);
            // // w_pci_32(bus, device, func, 0x0C, 0xFFFFFFFF);

            // // writes the first byte --> OK!
            // // w_pci_8(bus, device, func, 0x0C, 0xFF);

            // // k_printf("pci_write_word: 0x%x\n",
            // //          pci_write_word(bus, device, func, 3 * 4,
            // 0xFFFFFFFF));

            // // does not work
            // // w_pci_8(bus, device, func, 0x0D, 0xFF);
            // // w_pci_8(bus, device, func, PCI_LATENCY_TIMER, 0xAA);
            // // w_pci_8(bus, device, func, 3, 0xFF);

            // // pcibios_write_config_byte(bus, device, func, 0x0E, 64);

            // // pcibios_write_config_byte(pci_bus, pci_device_fn,
            // // PCI_LATENCY_TIMER,
            // //                           64);

            // k_printf("Test 2\n");
            // data = 0;
            // data = r_pci_32(bus, device, func, register_index);
            // k_printf("data 2: 0x%x\n", data);

            // // Cache Line Size
            // k_printf("Cache Line Size: 0x%x\n",
            //          pcibios_read_config_byte(bus, device, func, 0x0C));
            // // Latency Timer
            // k_printf("Latency Timer: 0x%x\n",
            //          pcibios_read_config_byte(bus, device, func, 0x0D));
            // // Header Type
            // k_printf("Header Type: 0x%x\n",
            //          pcibios_read_config_byte(bus, device, func, 0x0E));
            // // BIST
            // k_printf("BIST: 0x%x\n",
            //          pcibios_read_config_byte(bus, device, func, 0x0F));

            // k_printf("Test\n");
            // data = 0xFFFFFFFF;
            // data = pci_read_word(bus, device, func, 0);
            // k_printf("data: 0x%x\n", data);

            // pci_write_word(bus, device, func, 0, 0xFFFF);

            // k_printf("Test 2\n");
            // data = 0xFFFFFFFF;
            // data = pci_read_word(bus, device, func, 0);
            // k_printf("data 2: 0x%x\n", data);

            // data = pci_read_word(bus, device, func, 6);
            // k_printf("data: 0x%x\n", data);

            // // data = r_pci_32(bus, device, func, 2);
            // data = pci_read_word(bus, device, func, 2 * 4);
            // if (data != 0xffffffff) {

            //   u16int classcode = (data >> 24);
            //   u16int subclasscode = (data >> 16) & 0xFF;

            //   // 0x02 - network interface card - https://wiki.osdev.org/PCI
            //   k_printf("classcode: 0x%x", classcode);

            //   // 0x00 - Ethernet Controller - https://wiki.osdev.org/PCI
            //   k_printf(" subclasscode: 0x%x\n", subclasscode);
            // }

            // // data = r_pci_32(bus, device, func, 3);
            // data = pci_read_word(bus, device, func, 3 * 4);
            // if (data != 0xffffffff) {

            //   u16int bist = (data >> 24);
            //   u16int headertype = (data >> 16) & 0xFF;
            //   u16int latency = (data >> 8) & 0xFF;
            //   u16int cache_line_size = data & 0xFF;

            //   // bist is optional and hence not implemented by all cards
            //   k_printf("bist: 0x%x", bist);

            //   // header type register: https://wiki.osdev.org/PCI
            //   // Here is the layout of the Header Type register:
            //   // Bit 7 	Bits 6 to 0
            //   // MF 	Header Type
            //   //
            //   // MF - If MF = 1 Then this device has multiple functions.
            //   //
            //   // Header Type Codes:
            //   // 00h == Standard Header
            //   // 01h == PCI-to-PCI Bridge
            //   // 02h == CardBus Bridge
            //   k_printf(" headertype: 0x%x", headertype);

            //   k_printf(" latency: 0x%x (%dd)\n", latency, latency);

            //   pci_latency = latency;
            // }

            // // BAR0 - Base Address Register 0
            // // value is: 0xc001
            // // data = r_pci_32(bus, device, func, 4);
            // data = pci_read_word(bus, device, func, 4 * 4);
            // if (data != 0xffffffff) {
            //   k_printf("BAR0: 0x%x\n", data);
            // }

            // // BAR1 - Base Address Register 1
            // // value is: 0xfeb91000
            // // data = r_pci_32(bus, device, func, 5);
            // data = pci_read_word(bus, device, func, 5 * 4);
            // if (data != 0xffffffff) {
            //   k_printf("BAR1: 0x%x\n", data);
            // }

            // // BAR2 - Base Address Register 2
            // // data = r_pci_32(bus, device, func, 6);
            // data = pci_read_word(bus, device, func, 6 * 4);
            // if (data != 0xffffffff) {
            //   k_printf("BAR2: 0x%x\n", data);
            // }

            // // BAR3 - Base Address Register 3
            // // data = r_pci_32(bus, device, func, 7);
            // data = pci_read_word(bus, device, func, 7 * 4);
            // if (data != 0xffffffff) {
            //   k_printf("BAR3: 0x%x\n", data);
            // }

            // // BAR4 - Base Address Register 4
            // // data = r_pci_32(bus, device, func, 8);
            // data = pci_read_word(bus, device, func, 8 * 4);
            // if (data != 0xffffffff) {
            //   k_printf("BAR4: 0x%x\n", data);
            // }

            // // BAR5 - Base Address Register 5
            // // data = r_pci_32(bus, device, func, 9);
            // data = pci_read_word(bus, device, func, 9 * 4);
            // if (data != 0xffffffff) {
            //   k_printf("BAR5: 0x%x\n", data);
            // }
          }
        }
      }
    }
  }

  // this block will determine the ioaddr of the PCI NIC device
  // block scope

  // realtek8319Found = 0;
  if (realtek8319Found) {

    // pcibios_read_config_byte(pci_bus, pci_device_fn, PCI_INTERRUPT_LINE,
    //                          &pci_irq_line);

    // read the IRQ
    // value: 0x10b
    u32int pci_irq_line = r_pci_32(pci_bus, pci_device, pci_device_fn, 0x0F);
    // k_printf("pci_irq_line: 0x%x \n", pci_irq_line);
    // int irq = pci_irq_line >> 24;
    u32int irq = pci_irq_line & 0xFF;
    k_printf("irq: 0x%x (%dd)\n", irq, irq);

    // pcibios_read_config_dword(pci_bus, pci_device_fn, PCI_BASE_ADDRESS_0,
    //                           &pci_ioaddr);

    // read the ioaddr/base_address
    u32int pci_ioaddr = r_pci_32(pci_bus, pci_device, pci_device_fn, 4);
    k_printf("pci_ioaddr: 0x%x \n", pci_ioaddr);

    unsigned long ioaddr = pci_ioaddr & ~3;
    k_printf("ioaddr: 0x%x \n", ioaddr);

    // TODO: move to separate function
    //
    // Bring the chip out of low-power mode.
    //
    // Config1 = 0x52
    //
    // https://wiki.osdev.org/RTL8139
    //
    // Turning on the RTL8139 - Send 0x00 to the CONFIG_1 register (82 ==
    // 0x52) to set the LWAKE + LWPTN to active high. This should essentially
    // *power on* the device.
    // for (int i = 0; i < 999; i++) {

    // k_printf("starting chip ... %d\n", i);
    outb(0x00, ioaddr + Config1);
    k_printf("starting chip done.\n");
    //}

    /**/
    // https://wiki.osdev.org/RTL8139
    // enable bus mastering in the command register
    // Some BIOS may enable Bus Mastering at startup, but some versions
    // of qemu don't. You should thus be careful about this step.
    k_printf("BUS mastering ...\n");

    u16int command_register =
        pci_read_word(pci_bus, pci_device, pci_device_fn, 0x04);

    k_printf("BUS mastering command_register = %x\n", command_register);

    command_register |= 0x04;

    pci_write_word(pci_bus, pci_device, pci_device_fn, 0x04, command_register);

    command_register = pci_read_word(pci_bus, pci_device, pci_device_fn, 0x04);

    k_printf("BUS mastering command_register = %x\n", command_register);

    // wait
    for (int i = 0; i < 999999; i++) {
      // k_printf("w", i);
    }

    // if (command_register & 0x04) {

    //   k_printf("BUS mastering enabled. Command Register has bus mastering "
    //            "enabled already!\n");

    // } else {

    //   command_register |= 0x02;
    //   pci_write_word(bus, device, func, 0x04, command_register);
    //   k_printf("BUS mastering enabled.\n");
    // }

    // prepare mac address read
    int mac_address_index = 0;
    u32int mac_address[6];
    for (int i = 0; i < 6; i++) {
      mac_address[i] = 0;
    }

    // Read EEPROM
    //
    // TODO: move to separate function
    //
    // Read the MAC Addresses from the NIC's EEPROM memory chip
    // k_printf("read_eeprom() ...\n");

    int readEEPROMResult = read_eeprom(ioaddr, 0) != 0xffff;
    if (readEEPROMResult) {

      // k_printf("read_eeprom() done. true\n");

      // loop three times to read three int (= 32 bit)
      for (int i = 0; i < 3; i++) {

        //((u16 *)(dev->dev_addr))[i] = read_eeprom(ioaddr, i + 7);

        u16int data = read_eeprom(ioaddr, i + 7);

        // 0x5452 0x1200 0x5634
        // k_printf("MAC: 0x%x\n", data);

        mac_address[mac_address_index] = data & 0xFF;
        mac_address[mac_address_index + 1] = data >> 8;

        mac_address_index += 2;
      }

    } else {

      // k_printf("read_eeprom() done. false\n");

      // loop six times
      for (int i = 0; i < 6; i++) {

        // dev->dev_addr[i] = inb(ioaddr + MAC0 + i);
        // u16int data = inb(ioaddr + RTL8129_registers.MAC0 + i);
        u16int data = inb(ioaddr + i);

        // k_printf("MAC: 0x%x\n", data);

        mac_address_index += 1;
      }
    }

    // // DEBUG - print MAC Addresses
    // for (int i = 0; i < 5; i++) {
    //   k_printf("%2.2x:", dev->dev_addr[i]);
    // }

    // k_printf("%2.2x.\n", dev->dev_addr[i]);

    // DEBUG: print MAC Address
    k_printf("MAC: ");
    for (int i = 0; i < 6; i++) {
      k_printf("%x:", mac_address[i]);
    }
    k_printf("\n");

    /*
    // cannot set the latency because writing to the latency timer configuration
    //space register has no effect. The value does not change in that register
        // set latency
        if (pci_latency < 32) {

          k_printf("Updating latency timer to 64 ...\n");

          // pcibios_write_config_dword(pci_bus, pci_device, pci_device_fn,
       0x03,
          // 0xAAAAAAAA);

          uint16_t result =
              pci_write_word(pci_bus, pci_device, pci_device_fn, 0x00,
       0xAAAA); k_printf("result: 0x%x\n", result);

          // DEBUG: check the new value
          data = r_pci_32(pci_bus, pci_device, pci_device_fn, 0x00);
          if (data != 0xffffffff) {

            u16int a = (data >> 24);
            u16int b = (data >> 16) & 0xFF;
            u16int c = (data >> 8) & 0xFF;
            u16int d = data & 0xFF;

            k_printf("a: 0x%x", a);
            k_printf(" b: 0x%x", b);
            k_printf(" c: 0x%x ", c);
            k_printf(" d: 0x%x \n", d);

          } else {

            k_printf("FFFFFFFFF");
          }
        }
        */

    // software reset
    // https://wiki.osdev.org/RTL8139
    // Sending 0x10 to the Command register (0x37) will send the RTL8139 into a
    // software reset. Once that byte is sent, the RST bit must be checked to
    // make sure that the chip has finished the reset. If the RST bit is high
    // (1), then the reset is still in operation.

    // for (int i = 0; i < 9999; i++) {

    // ChipCmd is the Command Register 0x37 = 55
    // 0x10 == 0001 0000 == bit 5
    // k_printf("Reset the chip %d ...\n", i);
    outb(0x10, ioaddr + ChipCmd);
    while ((inb(ioaddr + ChipCmd) & 0x10) != 0) {
      k_printf("waiting for reset!\n");
    }
    k_printf("Reset done.\n");
    //}

    // wait
    for (int i = 0; i < 99999; i++) {
      // k_printf("w", i);
    }

    // for (int i = 0; i < 100; i++) {
    // enable receiver and transmitter
    // Sets the RE and TE bits high
    // k_printf("Enable receiver and transmitter %d...\n", i);
    // 0x0C = 1100 = bit 2 und bit 3
    outb(0x0C, ioaddr + ChipCmd);
    k_printf("Enable receiver and transmitter done.\n");
    //}

    // https://www.lowlevel.eu/wiki/RTL8139
    // Step 5
    // CR (Transmit Configuration Register, 0x40, 4 Bytes) und RCR
    // (Receive Configuration Register, 0x44, 4 Bytes) setzen.
    outl(0x03000700, ioaddr + TxConfig);
    outl(0x0000070a, ioaddr + RxConfig);

    // reset buffer
    char rx_buffer[8192 + 16];
    for (int i = 0; i < 8192 + 16; i++) {
      rx_buffer[i] = 0;
    }

    // set rx buffer = receive buffer
    // send uint32_t memory location to RBSTART (0x30 == 48d)
    outl(rx_buffer, ioaddr + RxBuf);

    // set IMR (Interupt Mask Register == 0x3C) and ISR (Interupt Service
    // Register == 0x3E) Sets the TOK and ROK bits high
    outw(0x0005, ioaddr + IntrMask);

    // configure the receive buffer
    // (1 << 7) is the WRAP bit, 0xf is AB+AM+APM+AAP
    outl(0xf | (1 << 7), ioaddr + 0x44);

    // for (int i = 0; i < 9999; i++) {
    //   k_printf("wait %d...\n", i);
    // }

    // tx = transmit buffer reset buffer
    int len = 256;
    unsigned char tx_buffer[len];
    for (int i = 0; i < len; i++) {
      tx_buffer[i] = 'A';
    }

    // int len = 42;

    // unsigned char tx_buffer[] = {
    //     0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x52, 0x54, 0x00, 0x12, 0x34,
    //     0x56, 0x08, 0x06, 0x00, 0x01, 0x08, 0x00, 0x06, 0x04, 0x00, 0x01,
    //     0x52, 0x54, 0x00, 0x12, 0x34, 0x56, 0x0a, 0x00, 0x02, 0x0f, 0x00,
    //     0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x02, 0x02};

    // First, copy the data to a physically contiguous chunk of memory
    // void *transfer_data = kmalloc_a(len);
    // void *phys_addr = virtual2phys(kpage_dir, transfer_data);
    // memcpy(transfer_data, data, len);

    // https: //
    // github.com/szhou42/osdev/blob/master/src/kernel/drivers/rtl8139.c
    k_printf("Sending...\n");

    // Four TXAD register, you must use a different one to send packet each
    // time(for example, use the first one, second... fourth and back to the
    // first)
    // TS = Transmit Status
    // AD = Address
    // D = ???
    //
    // TSAD = Transmit Start Registers = 32bit = Physical Address of data to
    // be sent
    u8int TSAD_array[4] = {0x20, 0x24, 0x28, 0x2C};

    // TSD - Transmit Status / Command Registers = 32bit
    u8int TSD_array[4] = {0x10, 0x14, 0x18, 0x1C};
    int tx_cur = 0;

    // Second, fill in physical address of data to TSD
    // outl((u32int)tx_buffer, ioaddr + TSAD_array[tx_cur]);
    outl(tx_buffer, ioaddr + TSAD_array[tx_cur]);

    // Fill the length to TSAD and start the transmission by setting the OWN
    // bit
    // to 0 Start https://wiki.osdev.org/RTL8139#Transmitting_Packets
    u32int status = 0;
    status |= len & 0x1FFF; // 0-12: Length
    // status |= 0 << 13;      // 13: OWN bit
    // status |=
    //    (0 & 0x3F) << 16;
    // 16-21: Early TX threshold (zero atm, TODO: check)
    // LOG("status = 0x%08x", status);
    // outd(card->IOBase + TSD0 + td * 4, status);

    outl(status, ioaddr + TSD_array[tx_cur]);

    // wait
    for (int i = 0; i < 99999; i++) {
      // k_printf("w", i);
    }

    u32int transmit_ok = inl(ioaddr + TSD_array[tx_cur]);
    while (transmit_ok & (1 << 15) == 0) {
      k_printf("Waiting for transmit_ok ...\n");
      transmit_ok = inl(ioaddr + TSD_array[tx_cur]);
    }
    k_printf("Waiting for transmit_ok done. transmit_ok = %d\n", transmit_ok);

    // transmit_ok = 41216 = 1010 0001 0000 0000

    // wait
    for (int i = 0; i < 99999; i++) {
      // k_printf("w", i);
    }

    tx_cur++;
    if (tx_cur > 3) {
      tx_cur = 0;
    }

    k_printf("Sending done.\n");

    for (int i = 0; i < 99999; i++) {
      // k_printf("wait %d...\n", i);
    }
  }

  return 0;
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

// pci_dev_t pci_find_devices(struct pci_device_id *ids, int index) {

//   struct pci_controller *hose;
//   pci_dev_t bdf;
//   int bus;

//   for (hose = pci_get_hose_head(); hose; hose = hose->next) {
//     for (bus = hose->first_busno; bus <= hose->last_busno; bus++) {
//       bdf = pci_hose_find_devices(hose, bus, ids, &index);
//       if (bdf != -1)
//         return bdf;
//     }
//   }

//   return -1;
// }