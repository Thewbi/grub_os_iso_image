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

static void test_use_frames_in_mb() {

  unsigned int total_frames = BITMAP_ELEMENT_COUNT * 32;

  reset_bitmap(bitmap, BITMAP_ELEMENT_COUNT);

  // map is empty
  for (int i = 0; i < BITMAP_ELEMENT_COUNT; i++) {
    assert_int_equal(0, bitmap[i]);
  }
  for (int i = 0; i < total_frames; i++) {
    assert_int_equal(0, get(bitmap, BITMAP_ELEMENT_COUNT, i));
  }

  // printf("A\n");

  unsigned int megabytes = 15;

  assert_int_equal(0, use_frames_in_mb(bitmap, BITMAP_ELEMENT_COUNT,
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

int main(int argc, char **argv) {

  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_array_size),
      cmocka_unit_test(test_reset_bitmap),
      cmocka_unit_test(test_set_bitmap),
      cmocka_unit_test(test_get_bitmap),
      cmocka_unit_test(test_use_frames_in_mb),
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}