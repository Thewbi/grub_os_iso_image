#ifndef PCI_H
#define PCI_H

#include "../common.h"
#include "../types.h"

u32int r_pci_32(u8int bus, u8int device, u8int func, u8int pcireg);

int init_pcilist(void);

#endif