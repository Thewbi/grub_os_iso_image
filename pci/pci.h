#ifndef PCI_H
#define PCI_H

#include "../common.h"
#include "../types.h"

/* 0x35-0x3b are reserved */
#define PCI_INTERRUPT_LINE	0x3c	/* 8 bits */
#define PCI_INTERRUPT_PIN	0x3d	/* 8 bits */

#define PCI_BASE_ADDRESS_0	0x10	/* 32 bits */
#define PCI_BASE_ADDRESS_1	0x14	/* 32 bits [htype 0,1 only] */
#define PCI_BASE_ADDRESS_2	0x18	/* 32 bits [htype 0 only] */
#define PCI_BASE_ADDRESS_3	0x1c	/* 32 bits */
#define PCI_BASE_ADDRESS_4	0x20	/* 32 bits */
#define PCI_BASE_ADDRESS_5	0x24	/* 32 bits */

#define PCI_LATENCY_TIMER	0x0d	/* 8 bits */

// Symbolic offsets to registers.
enum RTL8129_registers {
	MAC0=0,						// Ethernet hardware address.
	MAR0=8,						// Multicast filter.
	TxStatus0=0x10,				// Transmit status (Four 32bit registers).
	TxAddr0=0x20,				// Tx descriptors (also four 32bit).
	RxBuf=0x30, 
    RxEarlyCnt=0x34, 
    RxEarlyStatus=0x36,
	ChipCmd=0x37, 
    RxBufPtr=0x38, 
    RxBufAddr=0x3A,
	IntrMask=0x3C, 
    IntrStatus=0x3E,
	TxConfig=0x40, 
    RxConfig=0x44,
	Timer=0x48,					// A general-purpose counter
	RxMissed=0x4C,				// 24 bits valid, write clears.
	Cfg9346=0x50, 
    Config0=0x51, 
    Config1=0x52,
	FlashReg=0x54, 
    GPPinData=0x58, 
    GPPinDir=0x59, 
    MII_SMI=0x5A, 
    HltClk=0x5B,
	MultiIntr=0x5C, 
    TxSummary=0x60,
	MII_BMCR=0x62, 
    MII_BMSR=0x64, 
    NWayAdvert=0x66, 
    NWayLPAR=0x68,
	NWayExpansion=0x6A,
	// Undocumented registers, but required for proper operation.
	FIFOTMS=0x70,	// FIFO Test Mode Select
	CSCR=0x74,	    // Chip Status and Configuration Register.
	PARA78=0x78, 
    PARA7c=0x7c,	// Magic transceiver parameter register.
};

// EEPROM_Ctrl bits.
#define EE_SHIFT_CLK	0x04	// EEPROM shift clock.
#define EE_CS			0x08	// EEPROM chip select.
#define EE_DATA_WRITE	0x02	// EEPROM chip data in.
#define EE_WRITE_0		0x00
#define EE_WRITE_1		0x02
#define EE_DATA_READ	0x01	// EEPROM chip data out.
#define EE_ENB			(0x80 | EE_CS)

u32int r_pci_32(u8int bus, u8int device, u8int func, u8int pcireg);

int init_pcilist(void);

#endif