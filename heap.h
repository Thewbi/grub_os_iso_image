#ifndef HEAP_H
#define HEAP_H

#include "types.h"
#include "paging.h"

void k_free(void *ptr);

void *k_malloc(size_t request);

#endif