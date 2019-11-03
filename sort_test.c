#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include <cmocka.h>

#include "sort.h"

// clang-format off

/*
gcc -c sort.c
gcc -c sort_test.c -I/home/wbi/dev/cmocka-1.1.5/include
gcc -L /home/wbi/dev/cmocka-1.1.5-build/src -o "testmain" ./sort_test.o ./sort.o -lcmocka
./testmain

// BUILD AND LINK IN ONE GO
gcc -L /home/wbi/dev/cmocka-1.1.5-build/src -o "testmain" ./sort_test.o ./sort.o -lcmocka 

// In case you did execute 'make install' you can tell the linker to read the libraries from /usr/local/lib
gcc -L /usr/local/lib -o "testmain" ./sort_test.o ./sort.o -lcmocka
*/

// clang-format on

static void test_int_swap() {

  int data[5];

  for (int i = 0; i < 5; i++) {
    data[i] = 5 - i;
  }

  int_swap(data, 0, 1);

  assert_int_equal(4, data[0]);
  assert_int_equal(5, data[1]);
}

static void test_bubble_sort_simple() {

  const int array_size = 5;

  int data[array_size];
  data[0] = 67;
  data[1] = 89;
  data[2] = 45;
  data[3] = 17;
  data[4] = 39;

  // printf("Before:\n");
  // for (int i = 0; i < array_size; i++) {
  //   printf("%d\n", data[i]);
  // }

  bubblesort(data, array_size, sizeof(int), int_comparator, int_swap);

  // printf("After:\n");
  // for (int i = 0; i < array_size; i++) {
  //   printf("%d\n", data[i]);
  // }

  assert_int_equal(17, data[0]);
  assert_int_equal(39, data[1]);
  assert_int_equal(45, data[2]);
  assert_int_equal(67, data[3]);
  assert_int_equal(89, data[4]);
}

static void test_bubble_sort() {

  const int array_size = 30;

  int data[array_size];

  for (int i = 0; i < array_size; i++) {
    data[i] = i + 1;
  }

  shuffle(50, data, array_size, sizeof(int), int_swap);

  // printf("Before:\n");
  // for (int i = 0; i < array_size; i++) {
  //   printf("%d\n", data[i]);
  // }

  bubblesort(data, array_size, sizeof(int), int_comparator, int_swap);

  // printf("After:\n");
  // for (int i = 0; i < array_size; i++) {
  //   printf("%d\n", data[i]);
  // }

  for (int i = 0; i < array_size; i++) {
    assert_int_equal(i + 1, data[i]);
  }
}

static void test_shuffle() {

  int data[5];

  for (int i = 0; i < 5; i++) {
    data[i] = i + 1;
  }

  assert_true(shuffle(20, data, 5, sizeof(int), int_swap));

  // for (int i = 0; i < 5; i++) {
  //   printf("%d\n", data[i]);
  // }
}

// clang-format off
/*

rm ./testmain
gcc -c sort.c random.c
gcc -c sort_test.c -I/home/wbi/dev/cmocka-1.1.5/include
gcc -L /home/wbi/dev/cmocka-1.1.5-build/src -o "testmain" ./sort_test.o ./sort.o ./random.o -lcmocka
./testmain

*/
// clang-format on
int main(int argc, char **argv) {

  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_int_swap),
      cmocka_unit_test(test_bubble_sort),
      cmocka_unit_test(test_bubble_sort_simple),
      cmocka_unit_test(test_shuffle),
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}