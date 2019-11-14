#ifndef PAGING_H
#define PAGING_H

#include "bitmap.h"
#include "common.h"
#include "types.h"

void setup_page(uint32_t virtual_address);

void initialize_page_table(u32int *page_table_ptr,
                           uint32_t *page_table_frame_address_physical);

void dump_table(u32int *table_ptr);

#endif