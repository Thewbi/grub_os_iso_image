#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include <cmocka.h>

#include "bitmap.h"

static void test_array_size() { assert_int_equal(1601, BITMAP_ELEMENT_COUNT); }

static void test_reset_bitmap() {

  reset_bitmap(bitmap, BITMAP_ELEMENT_COUNT);

  // map is empty
  for (int i = 0; i < BITMAP_ELEMENT_COUNT; i++) {
    assert_int_equal(0, bitmap[i]);
  }
}

static void test_set_bitmap() {

  reset_bitmap(bitmap, BITMAP_ELEMENT_COUNT);

  // map is empty
  for (int i = 0; i < BITMAP_ELEMENT_COUNT; i++) {
    assert_int_equal(0, bitmap[i]);
  }

  // set first page
  assert_int_equal(0, set(bitmap, BITMAP_ELEMENT_COUNT, 0, 1));

  assert_int_equal(2147483648, bitmap[0]);

  // set second page
  assert_int_equal(0, set(bitmap, BITMAP_ELEMENT_COUNT, 1, 1));

  assert_int_equal(3221225472, bitmap[0]);

  // set page 16
  assert_int_equal(0, set(bitmap, BITMAP_ELEMENT_COUNT, 16, 1));

  assert_int_equal(3221258240, bitmap[0]);

  // reset first page
  assert_int_equal(0, set(bitmap, BITMAP_ELEMENT_COUNT, 0, 0));

  assert_int_equal(1073774592, bitmap[0]);

  // set page 32
  assert_int_equal(0, set(bitmap, BITMAP_ELEMENT_COUNT, 32, 1));

  assert_int_equal(2147483648, bitmap[1]);

  // set page that does not exist
  assert_int_equal(-1, set(bitmap, BITMAP_ELEMENT_COUNT, 7766554433, 1));
}

static void test_get_bitmap() {

  reset_bitmap(bitmap, BITMAP_ELEMENT_COUNT);

  // map is empty
  for (int i = 0; i < BITMAP_ELEMENT_COUNT; i++) {
    assert_int_equal(0, bitmap[i]);
  }

  assert_int_equal(0, get(bitmap, BITMAP_ELEMENT_COUNT, 0));

  // set first page
  assert_int_equal(0, set(bitmap, BITMAP_ELEMENT_COUNT, 0, 1));

  assert_int_equal(2147483648, bitmap[0]);

  assert_int_equal(1, get(bitmap, BITMAP_ELEMENT_COUNT, 0));
}

static void test_use_frames_in_bytes() {

  reset_bitmap(bitmap, BITMAP_ELEMENT_COUNT);

  // map is empty
  for (int i = 0; i < BITMAP_ELEMENT_COUNT; i++) {
    assert_int_equal(0, bitmap[i]);
  }

  // the get method also says all frames are empty
  unsigned int total_frames = BITMAP_ELEMENT_COUNT * 32;
  for (int i = 0; i < total_frames; i++) {
    assert_int_equal(0, get(bitmap, BITMAP_ELEMENT_COUNT, i));
  }

  // printf("A\n");

  unsigned int megabytes = 15;

  assert_int_equal(0, use_frames_in_bytes(bitmap, BITMAP_ELEMENT_COUNT,
                                          (megabytes * 1024 * 1024)));

  int frames = (megabytes * 1024 * 1024) / (1024 * 4);

  // bits from 0 to frames are set to 1
  for (int i = 0; i < frames; i++) {
    assert_int_equal(1, get(bitmap, BITMAP_ELEMENT_COUNT, i));
  }

  // printf("B\n");

  // the rest of the bits are set to 0
  for (int i = frames; i < total_frames; i++) {
    assert_int_equal(0, get(bitmap, BITMAP_ELEMENT_COUNT, i));
  }
}

static void test_next_free_frame() {

  unsigned int total_frames = BITMAP_ELEMENT_COUNT * 32;

  reset_bitmap(bitmap, BITMAP_ELEMENT_COUNT);

  // map is empty
  for (int i = 0; i < BITMAP_ELEMENT_COUNT; i++) {
    assert_int_equal(0, bitmap[i]);
  }

  // the get method also says all frames are empty
  for (int i = 0; i < total_frames; i++) {
    assert_int_equal(0, get(bitmap, BITMAP_ELEMENT_COUNT, i));
  }

  // printf("%u\n", bitmap[0]);

  // the first free index is 0
  assert_int_equal(0, next_free_frame_index(bitmap, BITMAP_ELEMENT_COUNT));

  // set the first frame used
  set(bitmap, BITMAP_ELEMENT_COUNT, 0, 1);
  assert_int_equal(1, get(bitmap, BITMAP_ELEMENT_COUNT, 0));

  // printf("%u\n", bitmap[0]);

  // the next free index is 1
  assert_int_equal(1, next_free_frame_index(bitmap, BITMAP_ELEMENT_COUNT));

  set(bitmap, BITMAP_ELEMENT_COUNT, 1, 1);
  set(bitmap, BITMAP_ELEMENT_COUNT, 2, 1);
  set(bitmap, BITMAP_ELEMENT_COUNT, 3, 1);
  set(bitmap, BITMAP_ELEMENT_COUNT, 4, 1);
  set(bitmap, BITMAP_ELEMENT_COUNT, 5, 1);

  // printf("%u\n", bitmap[0]);

  // the next free index is 1
  assert_int_equal(6, next_free_frame_index(bitmap, BITMAP_ELEMENT_COUNT));

  bitmap[0] = 0xFFFFFFFF;

  assert_int_equal(32, next_free_frame_index(bitmap, BITMAP_ELEMENT_COUNT));

  bitmap[1] = 0xFFFFFFFF;

  assert_int_equal(64, next_free_frame_index(bitmap, BITMAP_ELEMENT_COUNT));

  bitmap[2] = 0xFFFFFFFF;

  assert_int_equal(96, next_free_frame_index(bitmap, BITMAP_ELEMENT_COUNT));
}

static void test_use_frames_in_bytes_2() {

  unsigned int total_frames = BITMAP_ELEMENT_COUNT * 32;

  reset_bitmap(bitmap, BITMAP_ELEMENT_COUNT);

  // map is empty
  for (int i = 0; i < BITMAP_ELEMENT_COUNT; i++) {
    assert_int_equal(0, bitmap[i]);
  }

  // the get method also says all frames are empty
  for (int i = 0; i < total_frames; i++) {
    assert_int_equal(0, get(bitmap, BITMAP_ELEMENT_COUNT, i));
  }

  // reserve one frame, which will set the first bit
  use_frames_in_bytes(bitmap, BITMAP_ELEMENT_COUNT, 0x1000);

  // bit 0 is set
  assert_int_equal(1, get(bitmap, BITMAP_ELEMENT_COUNT, 0));
  assert_int_equal(0, get(bitmap, BITMAP_ELEMENT_COUNT, 1));

  // the next free bit is 1
  assert_int_equal(1, next_free_frame_index(bitmap, BITMAP_ELEMENT_COUNT));

  // reserve two frames, which will set the first and second bit
  use_frames_in_bytes(bitmap, BITMAP_ELEMENT_COUNT, 0x2000);

  // bit 1 is set
  assert_int_equal(1, get(bitmap, BITMAP_ELEMENT_COUNT, 0));
  assert_int_equal(1, get(bitmap, BITMAP_ELEMENT_COUNT, 1));
  assert_int_equal(0, get(bitmap, BITMAP_ELEMENT_COUNT, 2));

  // the next free bit is 2
  assert_int_equal(2, next_free_frame_index(bitmap, BITMAP_ELEMENT_COUNT));

  // reserve twelve frames, which will set the first and second bit
  use_frames_in_bytes(bitmap, BITMAP_ELEMENT_COUNT, 0x1000 * 12);

  // bit 0-11 are set for twelve frames
  assert_int_equal(1, get(bitmap, BITMAP_ELEMENT_COUNT, 0));
  assert_int_equal(1, get(bitmap, BITMAP_ELEMENT_COUNT, 1));
  assert_int_equal(1, get(bitmap, BITMAP_ELEMENT_COUNT, 2));
  assert_int_equal(1, get(bitmap, BITMAP_ELEMENT_COUNT, 3));
  assert_int_equal(1, get(bitmap, BITMAP_ELEMENT_COUNT, 4));
  assert_int_equal(1, get(bitmap, BITMAP_ELEMENT_COUNT, 5));
  assert_int_equal(1, get(bitmap, BITMAP_ELEMENT_COUNT, 6));
  assert_int_equal(1, get(bitmap, BITMAP_ELEMENT_COUNT, 7));
  assert_int_equal(1, get(bitmap, BITMAP_ELEMENT_COUNT, 8));
  assert_int_equal(1, get(bitmap, BITMAP_ELEMENT_COUNT, 9));
  assert_int_equal(1, get(bitmap, BITMAP_ELEMENT_COUNT, 10));
  assert_int_equal(1, get(bitmap, BITMAP_ELEMENT_COUNT, 11));

  // the next free bit is 12
  assert_int_equal(12, next_free_frame_index(bitmap, BITMAP_ELEMENT_COUNT));
}

int main(int argc, char **argv) {

  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_array_size),
      cmocka_unit_test(test_reset_bitmap),
      cmocka_unit_test(test_set_bitmap),
      cmocka_unit_test(test_get_bitmap),
      cmocka_unit_test(test_use_frames_in_bytes),
      cmocka_unit_test(test_next_free_frame),
      cmocka_unit_test(test_use_frames_in_bytes_2),
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}