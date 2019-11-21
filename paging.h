#ifndef PAGING_H
#define PAGING_H

#include "bitmap.h"
#include "common.h"
#include "types.h"

void setup_page(uint32_t virtual_address);

void initialize_page_table(u32int *page_table_ptr,
                           uint32_t *page_table_frame_address_physical);

void dump_table(u32int *table_ptr);

void page_fault_interrupt_handler(registers_t regs);

/**
 * @brief absolute positioning of the break (= end of heap memory area)
 * 
 * brk() sets the end of the data segment to the value specified by addr, 
 * when that value is reasonable, the system has enough memory, and the 
 * process does not exceed its maximum data size (see setrlimit(2)).
 * 
 * @see manpage of brk() (man brk)
 * 
 * @param addr set the break to this address
 * @return int On success, brk() returns zero.  On error, -1 is returned
 */
int brk(void *addr);

/**
 * @brief relative positioning of the break (= end of heap memory area)
 * 
 * sbrk() increments the program's data space by increment bytes.
 * Calling sbrk() with an increment of 0  can  be
 * used to find the current location of the program break.
 * 
 * @see manpage of sbrk() (man sbrk)
 * 
 * @param incr the amount of bytes to increment the break.
 * @return void* On success, sbrk() returns the previous program break. -1 on error.
 */
void *sbrk(int incr);


#endif