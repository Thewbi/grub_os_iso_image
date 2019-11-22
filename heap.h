#ifndef HEAP_H
#define HEAP_H

#include "types.h"
#include "paging.h"

void free(void *ptr);

void *malloc(size_t request);

#endif