// common.h -- Defines typedefs and some global functions.
//             From JamesM's kernel development tutorials.

#ifndef COMMON_H
#define COMMON_H

// this is a macro which calls the panic function and automatically
// specifies the line and the file of the MACRO usage
#define PANIC(msg) panic(msg, __FILE__, __LINE__);

#define USIGNED_INT_MAX 0xFFFFFFFF

// Some nice typedefs, to standardise sizes across platforms.
// These typedefs are written for 32-bit X86.
typedef unsigned int u32int;
typedef int s32int;
typedef unsigned short u16int;
typedef short s16int;
typedef unsigned char u8int;
typedef char s8int;

// https://wiki.osdev.org/I/O_Ports
// An I/O port is usually used as a technical term for a specific address on the
// x86's IO bus. This bus provides communication with devices in a fixed order
// and size, and was used as an alternative to memory access.
//
// https://stackoverflow.com/questions/3215878/what-are-in-out-instructions-in-x86-used-for
// The term "port" is coined in the intel manuals.

// outb - output a byte to a port
// with "b" meaning byte
void outb(u8int value, u16int port);

// outw - output a WORD (16 bit) to a port
// and "w" meaning word (16 bits).
void outw(u16int value, u16int port);

// outl - output a DWORD to a port
// and "l" meaning long (32 bits).
void outl(u32int value, u16int port);

// inb - read a byte from a port
u8int inb(u16int port);

// inw - read a word from a port
// "w" meaning word (16 bits)
u16int inw(u16int port);

// inl - read a DWORD from a port
// "l" meaning DWORD (32 bits)
u32int inl(u16int port);

// check if the bit BIT in FLAGS is set.
#define CHECK_FLAG(flags, bit) ((flags) & (1 << (bit)))

// video memory begins here.
#define VGA_ADDRESS 0xB8000

// VGA provides support for 16 colors */
#define BLACK 0
#define GREEN 2
#define RED 4
#define YELLOW 14
#define WHITE_COLOR 15

unsigned short *terminal_buffer;
unsigned int vga_index;

// Some screen stuff.
// The number of columns.
#define COLUMNS 80
// The number of lines.
#define LINES 24
// The attribute of an character.
#define ATTRIBUTE 7
// The video memory address.
#define VIDEO 0xB8000

// Variables.
// Save the X position.
int xpos;
// Save the Y position.
int ypos;
// Point to the video memory.
volatile unsigned char *video;

void k_cls(void);
void k_itoa(char *buf, int base, int d);
void k_putchar(int c);
void k_printf(const char *format, ...);
void k_memcpy(u8int *dest, const u8int *src, u32int len);
void k_memset(u8int *dest, u8int val, u32int len);
int k_strcmp(char *str1, char *str2);
char *k_strcpy(char *dest, const char *src);
char *k_strcat(char *dest, const char *src);
void k_print_float(float data);

extern void panic(const char *message, const char *file, u32int line);

#endif // COMMON_H
