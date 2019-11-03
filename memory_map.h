#ifndef MEMORY_MAP_H
#define MEMORY_MAP_H

#include "common.h"
#include "placement_memory.h"
#include "multiboot.h"

// free memory areas
extern memory_area_t free_memory_areas[10];
extern unsigned int free_memory_area_index;

int process_multiboot_memory_map(multiboot_info_t *mbi);
void dump_free_memory_map();

#endif