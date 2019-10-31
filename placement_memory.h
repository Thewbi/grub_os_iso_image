#ifndef PLACEMENT_MEMORY_H
#define PLACEMENT_MEMORY_H

typedef unsigned long long multiboot_uint64_t;

// used to describe memory areas
typedef struct memory_area {
  multiboot_uint64_t start;
  multiboot_uint64_t size;
} memory_area_t __attribute__((packed));

void init_placement_memory();
int contains(memory_area_t *lhs, memory_area_t *rhs);

#endif