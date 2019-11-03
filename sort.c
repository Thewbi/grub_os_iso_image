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
  unsigned int x0 = retrieve_pseudo_random(seed);

  for (int i = 0; i < shuffle_count; i++) {

    unsigned int first_random_int = 0;
    unsigned int second_random_int = 0;

    x0 = retrieve_pseudo_random(x0);

    first_random_int = (unsigned int)x0;

    x0 = retrieve_pseudo_random(x0);

    second_random_int = (unsigned int)x0;

    float first_random_uniform =
        (float)first_random_int / (float)USIGNED_INT_MAX;
    float second_random_uniform =
        (float)second_random_int / (float)USIGNED_INT_MAX;

    unsigned int first_index = first_random_uniform * (float)element_count;
    unsigned int second_index = second_random_uniform * (float)element_count;

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

// https://de.wikibooks.org/wiki/Algorithmen_und_Datenstrukturen_in_C/_Quicksort
void quicksort_internal(const void *data, unsigned int element_count,
                        unsigned int element_size,
                        int comparator_fptr(const void *, const void *),
                        void swap_fptr(const void *, unsigned int i,
                                       unsigned int j),
                        int first, int last) {

  // input parameters are valid - first is before last
  if (last - first < 1) {
    return;
  }

  unsigned int ptr_idx = first;

  // ptr = begin;
  void *ptr = data + element_size * ptr_idx;

  // split = begin + 1;
  unsigned int split_index = first + 1;

  // while (++ptr <= end) {
  while (++ptr_idx <= last) {

    // if (*ptr < *begin) {
    if (comparator_fptr(data + element_size * ptr_idx,
                        data + element_size * first) < 0) {

      // swap(ptr, split);
      swap_fptr(data, ptr_idx, split_index);

      //++split;
      split_index++;
    }
  }

  // swap(begin, split - 1);
  swap_fptr(data, first, (split_index - 1));

  // quicksort(begin, split - 1);
  quicksort_internal(data, 0, element_size, comparator_fptr, swap_fptr, first,
                     split_index - 1);

  //  quicksort(split, end);
  quicksort_internal(data, 0, element_size, comparator_fptr, swap_fptr,
                     split_index, last);
}

void quicksort(const void *data, unsigned int element_count,
               unsigned int element_size,
               int comparator_fptr(const void *, const void *),
               void swap_fptr(const void *, unsigned int i, unsigned int j)) {

  quicksort_internal(data, element_count, element_size, comparator_fptr,
                     swap_fptr, 0, element_count - 1);
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