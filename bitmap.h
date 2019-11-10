#ifndef BITMAP_H
#define BITMAP_H

#include "common.h"
#include "types.h"

// 200 MB of RAM
#define BITMAP_SIZE_FOR_MB 200

// Pages are assumed to be 4KB each. Convert MB to KB, then divide by 4
#define PAGE_AMOUNT BITMAP_SIZE_FOR_MB * 1024 / 4

// an 32 bit int can manage 32 pages at once, add one for module 
#define BITMAP_ELEMENT_COUNT PAGE_AMOUNT / 32 + 1

uint32_t bitmap[BITMAP_ELEMENT_COUNT];

void reset_bitmap(uint32_t * bitmap, unsigned int elements_in_array);

int set(uint32_t * bitmap, unsigned int elements_in_array, unsigned int frame_index, unsigned int value);

int get(uint32_t * bitmap, unsigned int elements_in_array, unsigned int frame_index);

int use_frames_in_mb(uint32_t * bitmap, unsigned int elements_in_array, unsigned int size_in_bytes);

#endif