#include "sort.h"

int shuffle(int shuffle_count, const void *data, int element_count,
            int element_size,
            void swap_fptr(const void *, unsigned int i, unsigned int j)) {

  // shuffle requires randomness and cannot work if no source of randomness is
  // available
  if (!is_rdrand_available()) {
    return 0;
  }

  unsigned int seed = retrieve_rdrand();
  unsigned int random_m = 123;

  unsigned int x0 = seed;
  unsigned int x1 = random_m;
  unsigned int x2 = 0;

  for (int i = 0; i < shuffle_count; i++) {

    unsigned int first_random_int = 0;
    unsigned int second_random_int = 0;

    x2 = retrieve_pseudo_random(x0, x1);
    x0 = x1;
    x1 = x2;

    first_random_int = (unsigned int)x2;
    // printf("first_random_int: %d\n", first_random_int);

    x2 = retrieve_pseudo_random(x0, x1);
    x0 = x1;
    x1 = x2;

    second_random_int = (unsigned int)x2;
    // printf("second_random_int: %d\n", second_random_int);

    float first_random_uniform =
        (float)first_random_int / (float)USIGNED_INT_MAX;
    float second_random_uniform =
        (float)second_random_int / (float)USIGNED_INT_MAX;

    // printf("first_random_uniform: %d\n", first_random_uniform);
    // printf("second_random_uniform: %d\n", second_random_uniform);

    unsigned int first_index = first_random_uniform * (float)element_count;
    unsigned int second_index = second_random_uniform * (float)element_count;

    // printf("first_index: %d\n", first_index);
    // printf("second_index: %d\n", second_index);

    swap_fptr(data, first_index, second_index);
  }

  return 1;
}

void bubblesort(const void *data, unsigned int element_count,
                unsigned int element_size,
                int comparator_fptr(const void *, const void *),
                void swap_fptr(const void *, unsigned int i, unsigned int j)) {

  for (unsigned int n = element_count; n > 1; --n) {
    for (unsigned int i = 0; i < n - 1; ++i) {
      if (comparator_fptr(data + element_size * i,
                          data + element_size * (i + 1)) > 0) {
        swap_fptr(data, i, i + 1);
      }
    }
  }
}

int int_comparator(const void *lhs_void_ptr, const void *rhs_void_ptr) {
  //   const mydata *p1 = (mydata *)v1;
  //   const mydata *p2 = (mydata *)v2;

  //   if (p1->id < p2->id)
  //       return -1;
  //   else if (p1->id > p2->id)
  //       return +1;
  //   else
  //       return 0;

  const int *lhs = (int *)lhs_void_ptr;
  const int *rhs = (int *)rhs_void_ptr;

  // printf("comp lhs:%d\n", *lhs);
  // printf("comp rhs:%d\n", *rhs);

  if (*lhs < *rhs) {
    return -1;
  } else if (*lhs > *rhs) {
    return +1;
  } else {
    return 0;
  }
}

void int_swap(const void *data, unsigned int i, unsigned int j) {

  if (i < 0 || j < 0) {
    return;
  }

  int *int_data = (int *)data;

  int temp = int_data[i];
  int_data[i] = int_data[j];
  int_data[j] = temp;
}