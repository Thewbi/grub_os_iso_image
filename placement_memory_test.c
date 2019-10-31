
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include <cmocka.h>

#include "placement_memory.h"

// clang-format off

// gcc -c placement_memory.c
// gcc -c placement_memory_test.c -I/home/wbi/dev/cmocka-1.1.5/include
// gcc -L /home/wbi/dev/cmocka-1.1.5-build/src -o "testmain" ./placement_memory_test.o ./placement_memory.o -lcmocka

// BUILD AND LINK IN ONE GO
// gcc -L /home/wbi/dev/cmocka-1.1.5-build/src -o "testmain" ./placement_memory_test.o ./placement_memory.o -lcmocka 

// In case you did execute 'make install' you can tell the linker to read the libraries from /usr/local/lib
// gcc -L /usr/local/lib -o "testmain" ./placement_memory_test.o ./placement_memory.o -lcmocka

// clang-format on

static void test_contains() {

  memory_area_t lhs;
  lhs.start = 0;
  lhs.size = 100;

  memory_area_t rhs;
  rhs.start = 10;
  rhs.size = 2;

  assert_true(contains(&lhs, &rhs));
}

static void test_contains_itself() {

  memory_area_t lhs;
  lhs.start = 0;
  lhs.size = 100;

  memory_area_t rhs;
  rhs.start = 10;
  rhs.size = 2;

  assert_true(contains(&lhs, &lhs));
}

int main(int argc, char **argv) {

  const UnitTest tests[] = {
      unit_test(test_contains),
      unit_test(test_contains_itself),
  };

  return run_tests(tests);
}