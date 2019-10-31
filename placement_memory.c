#include "placement_memory.h"

memory_area_t free_memory_areas[10];
unsigned int free_memory_area_index;

void init_placement_memory() {

  free_memory_area_index = 0;

  // initialize all areas to zero
  for (int i = 0; i < 10; i++) {

    free_memory_areas[i].start = 0;
    free_memory_areas[i].size = 0;
  }
}

/*
  // initialize all areas to zero
  memory_area_t used_memory_areas[10];
  unsigned int used_memory_areas_index = 0;
  for (int i = 0; i < 10; i++) {
    used_memory_areas[i].start = 0;
    used_memory_areas[i].size = 0;
  }
  */

// is rhs contained in lhs
// does lhs contain rhs
int contains(memory_area_t *lhs, memory_area_t *rhs) {

  int start_in =
      rhs->start >= lhs->start && rhs->start <= lhs->start + lhs->size;

  int end_in = rhs->start + rhs->size >= lhs->start &&
               rhs->start + rhs->size <= lhs->start + lhs->size;

  return start_in && end_in;
}