#include "bitmap.h"

void reset_bitmap(uint32_t *bitmap, unsigned int elements_in_array) {

  for (unsigned int i = 0; i < elements_in_array; i++) {

    bitmap[i] = 0;
  }
}

int set(uint32_t *bitmap, unsigned int elements_in_array,
        unsigned int frame_index, unsigned int value) {

  unsigned int element = frame_index / 32;

  if (element > elements_in_array) {
    return -1;
  }

  unsigned int bit_index = 31 - frame_index % 32;

  if (value) {
    bitmap[element] |= 1 << bit_index;
  } else {
    bitmap[element] &= ~(1UL << bit_index);
  }

  return 0;
}

int get(uint32_t *bitmap, unsigned int elements_in_array,
        unsigned int frame_index) {

  unsigned int element = frame_index / 32;

  if (element > elements_in_array) {
    return -1;
  }

  unsigned int bit_index = 31 - frame_index % 32;

  return (bitmap[element] & 1 << bit_index) > 0;
}

int use_frames_in_bytes(uint32_t *bitmap, unsigned int elements_in_array,
                        unsigned int size_in_bytes) {

  // k_printf("Using frames for the first %d bytes.\n", size_in_bytes);
  // k_printf("Using frames for the first %d MB.\n",
  //          (size_in_bytes / 1024 / 1024));

  unsigned int frame_count = size_in_bytes / 0x1000;

  // printf("Frame_Count: %d\n", frame_count);

  if ((elements_in_array * 32) < frame_count) {
    return -1;
  }

  for (unsigned int i = 0; i < frame_count; i++) {
    set(bitmap, elements_in_array, i, 1);
  }

  return 0;
}

int next_free_frame_index(uint32_t *bitmap, unsigned int elements_in_array) {

  for (unsigned int i = 0; i < elements_in_array; i++) {

    uint32_t idx = 0x80000000;

    // k_printf("Element: %d\n", i);

    for (int j = 0; j < 32; j++) {

      // printf("Element %u\n", bitmap[i]);
      // printf("Tester %u\n", idx >> j);

      // if the frame is free, return it's index
      if (!(bitmap[i] & (idx >> j))) {
        return i * 32 + j;
      }
    }
  }

  // no free frames left
  return -1;
}