#ifndef TYPES_H
#define TYPES_H

#define USIGNED_INT_MAX 0xFFFFFFFF

#define NULL 0

// Some nice typedefs, to standardise sizes across platforms.
// These typedefs are written for 32-bit X86.
typedef unsigned int u32int;
typedef int s32int;
typedef unsigned short u16int;
typedef short s16int;
typedef unsigned char u8int;
typedef char s8int;

typedef unsigned long long multiboot_uint64_t;
typedef unsigned int uint32_t;

typedef unsigned int size_t;

typedef struct registers
{
    u32int ds;                                     // Data segment selector
    u32int edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha.
    u32int int_no, err_code;                       // Interrupt number and error code (if applicable)
    u32int eip, cs, eflags, useresp, ss;           // Pushed by the processor automatically.
} registers_t;

// Enables registration of callbacks for interrupts or IRQs.
// For IRQs, to ease confusion, use the #defines above as the
// first parameter.
typedef void (*isr_t)(registers_t);

#endif