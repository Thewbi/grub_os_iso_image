#include "placement_memory.h"

memory_area_t free_memory_areas[10];
unsigned int free_memory_area_index;

void clear_placement_memory_array() {

  free_memory_area_index = 0;

  // initialize all areas to zero
  for (int i = 0; i < 10; i++) {

    free_memory_areas[i].start = 0;
    free_memory_areas[i].size = 0;
  }
}

// is rhs fully contained in lhs
// does lhs contain rhs
int completely_contains(memory_area_t *lhs, memory_area_t *rhs) {

  int start_in =
      rhs->start >= lhs->start && rhs->start <= lhs->start + lhs->size;

  int end_in = rhs->start + rhs->size >= lhs->start &&
               rhs->start + rhs->size <= lhs->start + lhs->size;

  return start_in && end_in;
}

int overlapp(memory_area_t *lhs, memory_area_t *rhs) {

  multiboot_uint64_t lhs_start = lhs->start;
  multiboot_uint64_t lhs_end = lhs->start + lhs->size - 1;

  multiboot_uint64_t rhs_start = rhs->start;
  multiboot_uint64_t rhs_end = rhs->start + rhs->size - 1;

  return lhs_start <= rhs_start && rhs_start <= lhs_end ||
         lhs_start <= rhs_end && rhs_end <= lhs_end;
}

unsigned int memory_areas_size() {

  for (int i = 0; i < MAX_MEMORY_AREAS_SIZE; i++) {

    if (free_memory_areas[i].start == 0 && free_memory_areas[i].size == 0) {
      return i;
    }
  }

  return MAX_MEMORY_AREAS_SIZE;
}

int is_free(memory_area_t *area) { return area->size == 0 && area->start == 0; }

// Outputs the free areas in the memory map
void dump_free_memory_map() {

  printf("Free Memory Areas:\n");

  for (int i = 0; i < free_memory_area_index; i++) {

    // // bugged - printf does not deal with two format strings correctly!
    // printf("Bugged: start = 0x%x size = 0x%x\n", free_memory_areas[i].start,
    //        free_memory_areas[i].size);

    // // fine
    // printf("Fine: start = 0x%x", free_memory_areas[i].start);
    // printf(" size = 0x%x", free_memory_areas[i].size);
    // printf("\n");

    // // fine
    // printf("Fine: start = %d", free_memory_areas[i].start);
    // printf(" size = %d", free_memory_areas[i].size);
    // printf("\n");

    multiboot_uint64_t end =
        free_memory_areas[i].start + free_memory_areas[i].size - 1;
    // float size = end - free_memory_areas[i].start;
    float size = free_memory_areas[i].size;

    char *unit = "B";

    if (size >= 1024.0f) {
      size /= 1024.0f;
      unit = "KB";
    }

    if (size >= 1024.0f) {
      size /= 1024.0f;
      unit = "MB";
    }

    // The definition for 1G of RAM in this OS is 1 GB = 1024 MB
    // The definition for 1 GB of hard drive space in this OS may vary. I do not
    // know yet, because harddrive access is not implemented yet
    if (size >= 1024.0f) {
      size /= 1024.0f;
      unit = "GB";
    }

    printf("[%d, ", free_memory_areas[i].start);
    printf("%d] ", end);
    printf("size: %.2f ", size);
    printf("%s\n", unit);
  }
}

// find the first block in the list that is large enough to contain the size of
// bytes requested. Reduce this block by the amount of bytes requested.
//
// Special Case: removes an entire block because the block is exactly as large
// as the bytes requested.
// -> Shift all blocks after the removed one down a position.
//
// Special Case: Does not fit into any available block
int allocate_area(multiboot_uint64_t size) {

  // if there are no free memory areas, return
  if (free_memory_area_index == 0) {
    return -1;
  }

  // if someone wants to allocate a memory area of size 0, return
  if (size == 0) {
    return -2;
  }

  // iterate over all free memory areas and find one that is large enought to
  // satisfy the request
  for (int i = 0; i < free_memory_area_index; i++) {

    if (free_memory_areas[i].size > size) {

      int allocation_start = free_memory_areas[i].start;

      // move the start of this area back to allocate the memory at the lowest
      // location possible
      free_memory_areas[i].start += size;
      free_memory_areas[i].size -= size;

      // return the address at which the user can access the free memory
      return allocation_start;

    } else if (free_memory_areas[i].size == size) {

      int allocation_start = free_memory_areas[i].start;

      // remove this free memory area
      for (int j = i; j < free_memory_area_index; j++) {

        free_memory_areas[j].start = free_memory_areas[j + 1].start;
        free_memory_areas[j].size = free_memory_areas[j + 1].size;
      }

      free_memory_area_index--;

      // return the address at which the user can access the free memory
      return allocation_start;
    }
  }

  // request for memory cannot be satisfied
  return -3;
}

// TODO: if two areas exactly are next to each other, merge them, shift the
// records down
// TODO: if two areas are connected by an inserted area, merge all three areas,
// shift the records down
int insert_area(multiboot_uint64_t start, multiboot_uint64_t size) {

  // if the array is full, return false
  if (free_memory_area_index == MAX_MEMORY_AREAS_SIZE) {
    return -1;
  }

  // if someone wants to insert a free memory area of size 0, return
  if (size == 0) {
    return -2;
  }

  // assumption: array is sorted: go through and find the correct index

  unsigned int correct_index = 0;
  while (start > free_memory_areas[correct_index].start &&
         correct_index < free_memory_area_index) {
    correct_index++;
  }

  // printf("correct_index: %d\n", correct_index);

  memory_area_t new_area;
  new_area.start = start;
  new_area.size = size;

  // check predecessor
  if ((correct_index > 0) &&
      overlapp(&free_memory_areas[correct_index - 1], &new_area)) {
    // printf("Predecessor collision\n");
    return -3;
  }

  // check successor
  if (!is_free(&free_memory_areas[correct_index])) {
    if (overlapp(&free_memory_areas[correct_index], &new_area)) {
      // printf("Successor collision\n");
      return -4;
    }
  }

  // printf("no collision\n");

  // merge with predecessor
  if (correct_index > 0) {

    multiboot_uint64_t pred_end = free_memory_areas[correct_index - 1].start +
                                  free_memory_areas[correct_index - 1].size - 1;

    // they share an edge
    if (pred_end + 1 == start) {

      // printf("shared edge successor\n");

      // make the successor larger
      free_memory_areas[correct_index - 1].size += size;

      // compute the new end of the successor
      multiboot_uint64_t end = free_memory_areas[correct_index - 1].start +
                               free_memory_areas[correct_index - 1].size - 1;

      // if the successor shares an edge with the predecessor, merge and shift
      if (free_memory_areas[correct_index].start == end + 1) {

        // printf("shared edge predecessor\n");

        // make the predecessor even larger
        free_memory_areas[correct_index - 1].size +=
            free_memory_areas[correct_index].size;

        // erase the successor
        for (int i = correct_index; i < free_memory_area_index; i++) {

          free_memory_areas[i].start = free_memory_areas[i + 1].start;
          free_memory_areas[i].size = free_memory_areas[i + 1].size;

          free_memory_area_index--;
        }
      }

      return 0;
    }
  }

  // merge with succesor
  // if the successor shares an edge with the predecessor, merge and shift
  if (free_memory_areas[correct_index].start == (start + size)) {

    // make the successor larger
    free_memory_areas[correct_index].start = start;
    free_memory_areas[correct_index].size += size;

    return 0;
  }

  // shift all content toward the end
  for (int i = free_memory_area_index + 1; i > correct_index; i--) {

    // printf("shift\n");

    free_memory_areas[i].start = free_memory_areas[i - 1].start;
    free_memory_areas[i].size = free_memory_areas[i - 1].size;
  }

  // insert
  free_memory_areas[correct_index].start = start;
  free_memory_areas[correct_index].size = size;

  // increase free_memory_area_index
  free_memory_area_index++;

  return 0;
}