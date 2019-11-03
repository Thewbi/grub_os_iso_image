#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include <cmocka.h>

#include "random.h"

static void test_is_rdrand_available() {

  int result = is_rdrand_available();

  assert_int_equal(1, result);
}

static void test_retrieve_rdrand() {

  for (int i = 0; i < 100; i++) {
    printf("random value: %d\n", retrieve_rdrand());
  }

  int firstRandomInteger = retrieve_rdrand();
  int secondRandomInteger = retrieve_rdrand();

  // theoretically the two integers could be equal but I hope this case will
  // never occur and make the test fail!
  assert_int_not_equal(firstRandomInteger, secondRandomInteger);
}

static void test_asm() { asmtest(); }

// clang-format off
/*
rm ./testmain
gcc -c random.c
gcc -c random_test.c -I/home/wbi/dev/cmocka-1.1.5/include
gcc -L /home/wbi/dev/cmocka-1.1.5-build/src -o "testmain" ./random_test.o ./random.o -lcmocka
./testmain
*/
// clang-format on
int main(int argc, char **argv) {

  const struct CMUnitTest tests[] = {
      // cmocka_unit_test(test_asm),
      cmocka_unit_test(test_is_rdrand_available),
      cmocka_unit_test(test_retrieve_rdrand),
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}