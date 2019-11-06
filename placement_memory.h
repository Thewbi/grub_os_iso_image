#ifndef PLACEMENT_MEMORY_H
#define PLACEMENT_MEMORY_H

#include "common.h"

// clang-format off

/*
This file contains code to help with using placement memory.
Placement memory allows the user to define free areas at user defined locations.
It also allows the user to allocate an amount of bytes from the declared free
locations.

This code helps the OS to keep track of which memory areas it could still use.
Whenever the OS uses memory, it should update the memory map using the function in this file
so it can remember which memory contains important data and which memory is still free.

This code is only usable before paging is enabled! Paging changes everything.

The assumption is, that the memory is never returned! This code does not contain
functionality to free allocated areas!

Usage:

// 1. Reset/initialize the array that backs the entire placement memory system

clear_placement_memory_array();

// 2. Define free memory areas that you can allocate from later

// 1000 byte block starting at 0
if (insert_area(0, 1000)) 
{
  printf("Inserting free memory area failed!");
  return;
}

// 9000 byte block starting at 3000
if (insert_area(3000, 9000)) 
{
  printf("Inserting free memory area failed!");
  return;
}

// 3. Allocate memory from the free memory areas

// allocate a 100 byte block
// if the allocation fails for any reason the return value is a negative error code
// if the allocation succeeds, the return value is the address at which the memory is available for the caller
int address = allocate_area(100);
if (address < 0)
{
  printf("Allocating memory failed!");
  return;
}

*/

// clang-format on

#define MAX_MEMORY_AREAS_SIZE 10

typedef unsigned long long multiboot_uint64_t;

// used to describe memory areas
typedef struct memory_area {
  multiboot_uint64_t start;
  multiboot_uint64_t size;
} memory_area_t __attribute__((packed));

// resets the array that is used to manage free memory areas so that all
// knowledge about existing free memory areas are erased
void clear_placement_memory_array();

// returns true if lhs completely covers rhs
int completely_contains(memory_area_t *lhs, memory_area_t *rhs);

// returns true if there is overlapp between lhs and rhs
int overlapp(memory_area_t *lhs, memory_area_t *rhs);

// returns the amount of free memory_areas
unsigned int memory_areas_size();

// inserts a free area
int insert_area(multiboot_uint64_t, multiboot_uint64_t);

// output the map
// void k_dump_free_memory_map();
void dump_free_memory_map();

// Tries to find the lowest of the free areas, that can completely satisfy the
// request for the amount of bytes to allocate and reduces the size of this free
// memory area.
//
// RETURN VALUE: If the allocation fails for any reason the return value is a
// negative error code. If the allocation succeeds, the return value is the
// address at which the memory is available for the caller
int allocate(multiboot_uint64_t size);

// given a start and a size, allocate that area of RAM
//
// Cases:
// 1. Used area does not overlap with any free area --> error
// 2. Used area does overlap with a free area but is too large for the free area
// --> error
// 3. Used area aligns exactly with the start of a free area --> makes the free
// area smaller at the start
// 4. Used area aligns exactly with the end of a free area --> makes the free
// area smaller at the end
// 5. Used area is exactly as large as a free area. --> The free area completely
// is removed from the array
// 6. Used area splits a free area into two peaces --> the array uses up a new
// element and has enough space --> ok
// 7. Used area splits a free area into two peaces --> the array uses up a new
// element and DOES NOT HAVE enough space --> error
int allocate_area(multiboot_uint64_t start, multiboot_uint64_t size);

// will allocate the entire area from start up to start+size.
// If subsections of this area are already used, will allocate the holes in
// between and merge sections.
int allocate_area_cover(multiboot_uint64_t start, multiboot_uint64_t size);

// retrieve an amount of bytes
// - search the lowest memory area that is large enough
// - automatically take away that memory from the array of free memory areas

#endif