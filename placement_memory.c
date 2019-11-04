#include "placement_memory.h"

memory_area_t free_memory_areas[10];
unsigned int free_memory_area_index;

void clear_placement_memory_array() {

  free_memory_area_index = 0;

  // initialize all areas to zero
  for (int i = 0; i < MAX_MEMORY_AREAS_SIZE; i++) {

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
void k_dump_free_memory_map() {

  k_printf("Free Memory Areas:\n");

  for (int i = 0; i < free_memory_area_index; i++) {

    // // bugged - printf does not deal with two format strings correctly!
    // k_printf("Bugged: start = 0x%x size = 0x%x\n",
    // free_memory_areas[i].start,
    //        free_memory_areas[i].size);

    // // fine
    // k_printf("Fine: start = 0x%x", free_memory_areas[i].start);
    // k_printf(" size = 0x%x", free_memory_areas[i].size);
    // k_printf("\n");

    // // fine
    // k_printf("Fine: start = %d", free_memory_areas[i].start);
    // k_printf(" size = %d", free_memory_areas[i].size);
    // k_printf("\n");

    multiboot_uint64_t end = 0;
    if (free_memory_areas[i].size > 0) {
      end = free_memory_areas[i].start + free_memory_areas[i].size - 1;
    }

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

    k_printf("[%d, ", free_memory_areas[i].start);
    k_printf("%d] ", end);
    // k_printf("size: %.2f ", size);
    k_print_float(size);
    k_printf("%s\n", unit);
  }
}

/*
void dump_free_memory_map() {

  printf("Free Memory Areas:\n");

  for (int i = 0; i < free_memory_area_index; i++) {

    multiboot_uint64_t end = 0;
    if (free_memory_areas[i].size > 0) {
      end = free_memory_areas[i].start + free_memory_areas[i].size - 1;
    }

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
    printf("%.2f ", size);
    printf("%s\n", unit);
  }
}
*/

// find the first block in the list that is large enough to contain the size of
// bytes requested. Reduce this block by the amount of bytes requested.
//
// Special Case: removes an entire block because the block is exactly as large
// as the bytes requested.
// -> Shift all blocks after the removed one down a position.
//
// Special Case: Does not fit into any available block
int allocate(multiboot_uint64_t size) {

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

int allocate_area(multiboot_uint64_t start, multiboot_uint64_t size) {

  // if the array is empty, return false
  if (free_memory_area_index == 0) {
    // printf("Array is empty\n");

    return -1;
  }

  // if someone wants to remove a free memory area of size 0, return
  if (size == 0) {
    // printf("Size is zero\n");

    return -2;
  }

  // printf("Looking for overlap\n");

  // find the first free area that overlaps
  int current_area_index = -1;
  multiboot_uint64_t current_area_end = -1;
  multiboot_uint64_t current_area_start = -1;
  memory_area_t *current_area;
  for (int i = 0; i < free_memory_area_index; i++) {

    current_area = &free_memory_areas[i];

    current_area_start = current_area->start;
    current_area_end = current_area->start + current_area->size - 1;

    if (start > current_area_end) {
      continue;
    }

    current_area_index = i;
    break;
  }

  // returned area does not overlap with any free area!
  if (current_area_index == -1) {
    return -3;
  }

  // printf("found overlap\n");

  multiboot_uint64_t end = start + size - 1;

  // printf("[%d, %d] [%d, %d]\n", current_area_start, current_area_end, start,
  //     end);

  if (current_area_start == start && current_area_end > end) {

    // printf("truly larger");

    // case: overlap at start + free area is truly larger than allocated area

    // make the area smaller from the start
    current_area->start += size;
    current_area->size -= size;

    return 0;
  }

  if (current_area_start < start && current_area_end == end) {

    // printf("truly smaller");

    // case: overlap at end + free area is truly larger than allocated area

    // make the area smaller from the end
    current_area->size -= size;

    return 0;
  }

  if (current_area_start == start && current_area_end == end) {

    // printf("complete overlap");

    // case: areas are equal, the free area is erased

    for (int i = current_area_index; i < free_memory_area_index; i++) {

      free_memory_areas[i].start = free_memory_areas[i + 1].start;
      free_memory_areas[i].size = free_memory_areas[i + 1].size;
    }

    free_memory_area_index--;

    return 0;
  }

  if (current_area_start < start && current_area_end > end) {

    // printf("complete enclosed, split in two\n");

    // check if there is enough space to take up another area which is created
    // by the split
    if (free_memory_area_index == MAX_MEMORY_AREAS_SIZE) {

      return -4;
    }

    // make the current area smaller
    multiboot_uint64_t original_size = current_area->size;
    current_area->size = start - current_area->start;

    // shift to the right
    for (int i = free_memory_area_index; i > current_area_index; i--) {

      free_memory_areas[i].start = free_memory_areas[i - 1].start;
      free_memory_areas[i].size = free_memory_areas[i - 1].size;
    }

    // insert the second part of the split area
    free_memory_areas[current_area_index + 1].start = end + 1;
    free_memory_areas[current_area_index + 1].size =
        original_size - size - current_area->size;

    free_memory_area_index++;

    return 0;
  }

  // printf("No case matches!\n");

  return -5;
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

  // printf("A free_memory_area_index: %d\n", free_memory_area_index);

  unsigned int correct_index = 0;
  while (start > free_memory_areas[correct_index].start &&
         correct_index < free_memory_area_index) {
    correct_index++;
  }

  // printf("correct_index: %d\n", correct_index);

  // printf("B free_memory_area_index: %d\n", free_memory_area_index);

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

  // printf("C free_memory_area_index: %d\n", free_memory_area_index);
  // printf("correct_index: %d\n", correct_index);

  // printf("no collision\n");

  // merge with predecessor
  if (correct_index > 0) {

    multiboot_uint64_t pred_end = free_memory_areas[correct_index - 1].start +
                                  free_memory_areas[correct_index - 1].size - 1;

    // they share an edge
    if (pred_end + 1 == start) {

      // printf("shared edge successor\n");
      // printf("merge with successor\n");

      // make the successor larger
      free_memory_areas[correct_index - 1].size += size;

      // compute the new end of the successor
      multiboot_uint64_t end = free_memory_areas[correct_index - 1].start +
                               free_memory_areas[correct_index - 1].size - 1;

      // if the successor shares an edge with the predecessor, merge and shift
      if (free_memory_areas[correct_index].start == end + 1) {

        // printf("shared edge predecessor\n");
        // printf("merge with predecessor\n");

        // make the predecessor even larger
        free_memory_areas[correct_index - 1].size +=
            free_memory_areas[correct_index].size;

        // erase the successor
        for (int i = correct_index; i < free_memory_area_index; i++) {

          free_memory_areas[i].start = free_memory_areas[i + 1].start;
          free_memory_areas[i].size = free_memory_areas[i + 1].size;
        }

        // printf("decrement\n");
        free_memory_area_index--;
      }

      return 0;
    }
  }

  // printf("D free_memory_area_index: %d\n", free_memory_area_index);

  // merge with succesor
  // if the successor shares an edge with the predecessor, merge and shift
  if (free_memory_areas[correct_index].start == (start + size)) {

    // printf("merge with succesor\n");

    // make the successor larger
    free_memory_areas[correct_index].start = start;
    free_memory_areas[correct_index].size += size;

    // // shift all content toward the end
    // for (int i = free_memory_area_index + 1; i > correct_index; i--) {

    //   // printf("shift\n");

    //   free_memory_areas[i].start = free_memory_areas[i - 1].start;
    //   free_memory_areas[i].size = free_memory_areas[i - 1].size;
    // }

    return 0;
  }

  // printf("E free_memory_area_index: %d\n", free_memory_area_index);

  // printf("F free_memory_area_index: %d\n", free_memory_area_index);

  // insert
  free_memory_areas[correct_index].start = start;
  free_memory_areas[correct_index].size = size;

  // increase free_memory_area_index
  // printf("increment %d to ", free_memory_area_index);
  free_memory_area_index++;
  // printf("%d\n", free_memory_area_index);

  return 0;
}