#ifndef SORT_H
#define SORT_H

#include "common.h"
#include "random.h"

// RETURN: false, if the shuffle failed
int shuffle(int shuffle_count, const void *data, int element_count, 
            int element_size, void swap_fptr(const void *, unsigned int i, unsigned int j));

void bubblesort(const void *data, unsigned int element_count, unsigned int element_size,
                int comparator_fptr(const void *, const void *),
                void swap_fptr(const void *, unsigned int i, unsigned int j));

// standard implementations for an array of int
int int_comparator(const void *lhs_void_ptr, const void *rhs_void_ptr);
void int_swap(const void *data, unsigned int i, unsigned int j);

void quicksort(const void *data, unsigned int element_count,
               unsigned int element_size,
               int comparator_fptr(const void *, const void *),
               void swap_fptr(const void *, unsigned int i, unsigned int j));

#endif