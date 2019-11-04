
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include <cmocka.h>

#include "placement_memory.h"

// clang-format off

/*
rm ./testmain
gcc -c placement_memory.c
gcc -c placement_memory_test.c -I/home/wbi/dev/cmocka-1.1.5/include
gcc -L /home/wbi/dev/cmocka-1.1.5-build/src -o "testmain" ./placement_memory_test.o ./placement_memory.o -lcmocka

// BUILD AND LINK IN ONE GO
// gcc -L /home/wbi/dev/cmocka-1.1.5-build/src -o "testmain" ./placement_memory_test.o ./placement_memory.o -lcmocka 

// In case you did execute 'make install' you can tell the linker to read the libraries from /usr/local/lib
// gcc -L /usr/local/lib -o "testmain" ./placement_memory_test.o ./placement_memory.o -lcmocka
*/

// clang-format on

extern memory_area_t free_memory_areas[10];
extern unsigned int free_memory_area_index;

// Test a large area lhs contains a smaller area rhs completely
static void test_contains_A() {

  memory_area_t lhs;
  lhs.start = 0;
  lhs.size = 100;

  memory_area_t rhs;
  rhs.start = 10;
  rhs.size = 2;

  assert_true(completely_contains(&lhs, &rhs));
}

static void test_contains_B() {

  memory_area_t lhs;
  lhs.start = 100;
  lhs.size = 100;

  memory_area_t rhs;
  rhs.start = 50;
  rhs.size = 100;

  assert_false(completely_contains(&lhs, &rhs));
}

// Test that an area contains itself
static void test_contains_itself() {

  memory_area_t lhs;
  lhs.start = 0;
  lhs.size = 100;

  memory_area_t rhs;
  rhs.start = 10;
  rhs.size = 2;

  assert_true(completely_contains(&lhs, &lhs));
}

// Test that an area contains itself
static void test_overlapp() {

  memory_area_t lhs;
  lhs.start = 100;
  lhs.size = 100;

  memory_area_t rhs;
  rhs.start = 50;
  rhs.size = 100;

  assert_true(overlapp(&lhs, &lhs));
}

// Test setup routine
static void test_clear_placement_memory_array() {

  clear_placement_memory_array();

  // there is no memory area
  assert_int_equal(0, memory_areas_size());
  assert_int_equal(0, memory_areas_size());
}

// Test the allocation on a completely free area
static void test_kalloc_none_free() {

  // after the clear, there are no free memory areas left!
  clear_placement_memory_array();

  // request 100 bytes, this will fail because there is nothing free
  assert_int_equal(-1, allocate(100));
}

// Test the allocation on a completely free area
static void test_kalloc_zero() {

  // after the clear, there are no free memory areas left!
  clear_placement_memory_array();

  // insert 1000 bytes, 0 means, the insert did work
  assert_int_equal(0, insert_area(0, 1000));

  // request 100 bytes, this will fail because there is nothing free
  assert_int_equal(-2, allocate(0));
}

// Test the allocation on a completely free area
static void test_kalloc_all_free() {

  // after the clear, there are no free memory areas left!
  clear_placement_memory_array();

  // there is no memory area
  assert_int_equal(0, free_memory_area_index);
  assert_int_equal(0, memory_areas_size());

  // insert 1000 bytes, 0 means, the insert did work
  assert_int_equal(0, insert_area(0, 1000));

  // there is only a single memory area
  assert_int_equal(1, free_memory_area_index);
  assert_int_equal(1, memory_areas_size());

  // request 100 bytes
  assert_in_range(allocate(100), 0, 999);

  // it allocated at the lowest possible area which is zero and it reserved 100
  // bytes
  assert_int_equal(100, free_memory_areas[0].start);
  assert_int_equal(900, free_memory_areas[0].size);

  // there is only a single memory area
  assert_int_equal(1, free_memory_area_index);
  assert_int_equal(1, memory_areas_size());
}

// Test an allocation that consumes all free memory
static void test_kalloc_consume_all() {

  // after the clear, there are no free memory areas left!
  clear_placement_memory_array();

  // there is no memory area
  assert_int_equal(0, free_memory_area_index);
  assert_int_equal(0, memory_areas_size());

  // insert 1000 bytes, 0 means, the insert did work
  assert_int_equal(0, insert_area(0, 1000));

  // there is only a single memory area
  assert_int_equal(1, free_memory_area_index);
  assert_int_equal(1, memory_areas_size());

  // request 1000 bytes, this uses up the entire free memory block
  assert_in_range(allocate(1000), 0, 999);

  // there is no memory area
  assert_int_equal(0, free_memory_area_index);
  assert_int_equal(0, memory_areas_size());
}

// Test two allocations next to each other
static void test_two_allocations() {

  clear_placement_memory_array();

  // there is no memory area
  assert_int_equal(0, free_memory_area_index);
  assert_int_equal(0, memory_areas_size());

  // insert 1000 bytes, 0 means, the insert did work
  assert_int_equal(0, insert_area(0, 1000));

  // there is only a single memory area
  assert_int_equal(1, free_memory_area_index);
  assert_int_equal(1, memory_areas_size());

  // request 100 bytes
  assert_in_range(allocate(100), 0, 999);

  // it allocated at the lowest possible area which is zero and it reserved 100
  // bytes
  assert_int_equal(100, free_memory_areas[0].start);
  assert_int_equal(900, free_memory_areas[0].size);

  // there is only a single memory area
  assert_int_equal(1, free_memory_area_index);
  assert_int_equal(1, memory_areas_size());

  // request 100 bytes
  assert_in_range(allocate(100), 100, 999);

  // it allocated at the lowest possible area which is 100 and it reserved 100
  // bytes. But both areas are merged into one. The index is 0
  assert_int_equal(200, free_memory_areas[0].start);
  assert_int_equal(800, free_memory_areas[0].size);

  // there is only a single memory area
  assert_int_equal(1, free_memory_area_index);
  assert_int_equal(1, memory_areas_size());
}

// Test an allocation that cannot be served
static void test_kalloc_not_enough_memory() {

  // after the clear, there are no free memory areas left!
  clear_placement_memory_array();

  // there is no memory area
  assert_int_equal(0, free_memory_area_index);
  assert_int_equal(0, memory_areas_size());

  // insert 1000 bytes, 0 means, the insert did work
  assert_int_equal(0, insert_area(0, 1000));

  // there is only a single memory area
  assert_int_equal(1, free_memory_area_index);
  assert_int_equal(1, memory_areas_size());
  assert_int_equal(0, free_memory_areas[0].start);
  assert_int_equal(1000, free_memory_areas[0].size);

  // request 2000 bytes. This amount of memory cannot be served!
  assert_int_equal(-3, allocate(2000));

  // there is only a single memory area
  assert_int_equal(1, free_memory_area_index);
  assert_int_equal(1, memory_areas_size());
  assert_int_equal(0, free_memory_areas[0].start);
  assert_int_equal(1000, free_memory_areas[0].size);
}

// Test free, not large enough are at the start
static void test_kalloc_too_small() {

  clear_placement_memory_array();

  // there is no memory area
  assert_int_equal(0, free_memory_area_index);
  assert_int_equal(0, memory_areas_size());

  // first 10 byte are free, then there is a 100 byte block
  insert_area(10, 200);

  // there is only a single memory area
  assert_int_equal(1, free_memory_area_index);
  assert_int_equal(1, memory_areas_size());
  assert_int_equal(10, free_memory_areas[0].start);
  assert_int_equal(200, free_memory_areas[0].size);

  // request 100 bytes
  assert_in_range(allocate(100), 10, 199);

  // there is only a single memory area
  assert_int_equal(1, free_memory_area_index);
  assert_int_equal(1, memory_areas_size());
  assert_int_equal(110, free_memory_areas[0].start);
  assert_int_equal(100, free_memory_areas[0].size);
}

// Test free, not large enough are at the start
static void test_kalloc_second_area_serves() {

  clear_placement_memory_array();

  // there is no memory area
  assert_int_equal(0, free_memory_area_index);
  assert_int_equal(0, memory_areas_size());

  // first 10 byte are free, then there is a 100 byte block
  insert_area(10, 50);

  // there is only a single memory area
  assert_int_equal(1, free_memory_area_index);
  assert_int_equal(1, memory_areas_size());
  assert_int_equal(10, free_memory_areas[0].start);
  assert_int_equal(50, free_memory_areas[0].size);

  // first 10 byte are free, then there is a 100 byte block
  insert_area(100, 100);

  // there is only a single memory area
  assert_int_equal(2, free_memory_area_index);
  assert_int_equal(2, memory_areas_size());
  assert_int_equal(10, free_memory_areas[0].start);
  assert_int_equal(50, free_memory_areas[0].size);
  assert_int_equal(100, free_memory_areas[1].start);
  assert_int_equal(100, free_memory_areas[1].size);

  // request 100 bytes
  assert_in_range(allocate(70), 100, 199);

  // there is only a single memory area
  assert_int_equal(2, free_memory_area_index);
  assert_int_equal(2, memory_areas_size());
  assert_int_equal(10, free_memory_areas[0].start);
  assert_int_equal(50, free_memory_areas[0].size);
  assert_int_equal(170, free_memory_areas[1].start);
  assert_int_equal(30, free_memory_areas[1].size);
}

// Test Free large enough area at the start
static void test_insert_into_empty() {

  clear_placement_memory_array();

  // there is no memory area
  assert_int_equal(0, free_memory_area_index);
  assert_int_equal(0, memory_areas_size());

  // insert 1000 bytes, 0 means, the insert did work
  assert_int_equal(0, insert_area(0, 1000));

  // there is only a single memory area
  assert_int_equal(1, free_memory_area_index);
  assert_int_equal(1, memory_areas_size());

  assert_int_equal(0, free_memory_areas[0].start);
  assert_int_equal(1000, free_memory_areas[0].size);
}

// Test Free large enough area at the start
static void test_insert_area_already_used() {

  clear_placement_memory_array();

  // there is no memory area
  assert_int_equal(0, free_memory_area_index);
  assert_int_equal(0, memory_areas_size());

  // insert a 1000 byte block at the start
  insert_area(0, 1000);

  // there is only a single memory area
  assert_int_equal(1, free_memory_area_index);
  assert_int_equal(1, memory_areas_size());

  // this memory are is already used! Return value -2 signals a predecessor
  // collision case
  assert_int_equal(-3, insert_area(100, 100));

  // there is only a single memory area
  assert_int_equal(1, free_memory_area_index);
  assert_int_equal(1, memory_areas_size());
}

static void test_insert_area_already_used_successor_collision() {

  clear_placement_memory_array();

  // there is no memory area
  assert_int_equal(0, free_memory_area_index);
  assert_int_equal(0, memory_areas_size());

  // insert a 100 byte block at the start
  insert_area(100, 100);

  // there is only a single memory area
  assert_int_equal(1, free_memory_area_index);
  assert_int_equal(1, memory_areas_size());

  // this memory are is already used! Return value -2 signals a predecessor
  // collision case
  assert_int_equal(-4, insert_area(50, 100));

  // there is only a single memory area
  assert_int_equal(1, free_memory_area_index);
  assert_int_equal(1, memory_areas_size());
}

static void test_insert_area_already_used_last_collision() {

  clear_placement_memory_array();

  // there is no memory area
  assert_int_equal(0, free_memory_area_index);
  assert_int_equal(0, memory_areas_size());

  // insert a 100 byte block at the start
  insert_area(0, 100);

  // there is only a single memory area
  assert_int_equal(1, free_memory_area_index);
  assert_int_equal(1, memory_areas_size());

  // this memory are is already used! Return value -2 signals a predecessor
  // collision case
  assert_int_equal(-3, insert_area(50, 100));

  // there is only a single memory area
  assert_int_equal(1, free_memory_area_index);
  assert_int_equal(1, memory_areas_size());
}

static void test_insert_area_already_used_predecessor_collision() {

  clear_placement_memory_array();

  // there is no memory area
  assert_int_equal(0, free_memory_area_index);
  assert_int_equal(0, memory_areas_size());

  // insert a 100 byte block at the start
  assert_int_equal(0, insert_area(0, 100));

  // there is only a single memory area
  assert_int_equal(1, free_memory_area_index);
  assert_int_equal(1, memory_areas_size());

  // insert a 100 byte block at 200
  assert_int_equal(0, insert_area(200, 100));

  // there is only a single memory area
  assert_int_equal(2, free_memory_area_index);
  assert_int_equal(2, memory_areas_size());

  // this memory are is already used! Return value -2 signals a predecessor
  // collision case
  assert_int_equal(-4, insert_area(150, 100));

  // there is only a single memory area
  assert_int_equal(2, free_memory_area_index);
  assert_int_equal(2, memory_areas_size());
}

void test_insert_merge_successor() {

  clear_placement_memory_array();

  // there is no memory area
  assert_int_equal(0, free_memory_area_index);
  assert_int_equal(0, memory_areas_size());

  // insert a 100 byte block at the start
  assert_int_equal(0, insert_area(0, 100));

  // there is only a single memory area
  assert_int_equal(1, free_memory_area_index);
  assert_int_equal(1, memory_areas_size());

  // insert a 100 byte block at the start
  assert_int_equal(0, insert_area(100, 100));

  // dump_free_memory_map();

  // there is still only a single memory area, because the area have been merged
  assert_int_equal(1, free_memory_area_index);
  assert_int_equal(1, memory_areas_size());
}

void test_insert_merge_predecessor() {

  clear_placement_memory_array();

  // there is no memory area
  assert_int_equal(0, free_memory_area_index);
  assert_int_equal(0, memory_areas_size());

  // insert a 100 byte block at 100
  assert_int_equal(0, insert_area(100, 100)); // [100, 199]

  // there is only a single memory area
  assert_int_equal(1, free_memory_area_index);
  assert_int_equal(1, memory_areas_size());

  // insert a 100 byte block at the start
  assert_int_equal(0, insert_area(0, 100)); // [0, 99]

  // there is still only a single memory area, because the area have been merged
  assert_int_equal(1, free_memory_area_index);
  assert_int_equal(1, memory_areas_size());
}

void test_insert_merge_both_sides() {

  clear_placement_memory_array();

  // there is no memory area
  assert_int_equal(0, free_memory_area_index);
  assert_int_equal(0, memory_areas_size());

  // insert a 100 byte block at the start
  assert_int_equal(0, insert_area(0, 100));

  // there is only a single memory area
  assert_int_equal(1, free_memory_area_index);
  assert_int_equal(1, memory_areas_size());

  // insert a 100 byte block at the start
  assert_int_equal(0, insert_area(200, 100));

  // there is only a single memory area
  assert_int_equal(2, free_memory_area_index);
  assert_int_equal(2, memory_areas_size());

  assert_int_equal(0, insert_area(100, 100));

  // there is still only a single memory area, because the area have been merged
  assert_int_equal(1, free_memory_area_index);
  assert_int_equal(1, memory_areas_size());
}

void test_allocate_area_no_free_areas() {

  clear_placement_memory_array();

  // there is no free memory area
  assert_int_equal(0, free_memory_area_index);
  assert_int_equal(0, memory_areas_size());

  assert_int_equal(-1, allocate_area(100, 100));
}

void test_allocate_area_size_zero() {

  clear_placement_memory_array();

  // there is no free memory area
  assert_int_equal(0, free_memory_area_index);
  assert_int_equal(0, memory_areas_size());

  // insert a 100 byte block at the start
  assert_int_equal(0, insert_area(0, 100));

  // there is only a single memory area
  assert_int_equal(1, free_memory_area_index);
  assert_int_equal(1, memory_areas_size());

  assert_int_equal(-2, allocate_area(100, 0));
}

void test_allocate_area_no_overlap_with_free_area() {

  clear_placement_memory_array();

  // there is no free memory area
  assert_int_equal(0, free_memory_area_index);
  assert_int_equal(0, memory_areas_size());

  // insert a 100 byte block at the start
  assert_int_equal(0, insert_area(0, 100));
  assert_int_equal(0, insert_area(200, 100));

  // there is only a single memory area
  assert_int_equal(2, free_memory_area_index);
  assert_int_equal(2, memory_areas_size());

  assert_int_equal(-3, allocate_area(500, 100));
}

// Used area aligns exactly with the start of a free area --> makes the free
// area smaller at the start
void test_allocate_area_overlaps_with_start_free_area() {

  clear_placement_memory_array();

  // there is no free memory area
  assert_int_equal(0, free_memory_area_index);
  assert_int_equal(0, memory_areas_size());

  // insert a 100 byte block at the start
  assert_int_equal(0, insert_area(0, 100));
  assert_int_equal(0, insert_area(200, 100));

  // there are two free memory areas
  assert_int_equal(2, free_memory_area_index);
  assert_int_equal(2, memory_areas_size());

  assert_int_equal(0, allocate_area(200, 20));

  // there are still two memory areas
  assert_int_equal(2, free_memory_area_index);
  assert_int_equal(2, memory_areas_size());

  assert_int_equal(0, free_memory_areas[0].start);
  assert_int_equal(100, free_memory_areas[0].size);

  assert_int_equal(220, free_memory_areas[1].start);
  assert_int_equal(80, free_memory_areas[1].size);
}

// Used area aligns exactly with the start of a free area --> makes the free
// area smaller at the start
void test_allocate_area_overlaps_with_end_free_area() {

  clear_placement_memory_array();

  // there is no free memory area
  assert_int_equal(0, free_memory_area_index);
  assert_int_equal(0, memory_areas_size());

  // insert a 100 byte block at the start
  assert_int_equal(0, insert_area(0, 100));   // [0, 99]
  assert_int_equal(0, insert_area(200, 100)); // [200, 299]

  // there are two free memory areas
  assert_int_equal(2, free_memory_area_index);
  assert_int_equal(2, memory_areas_size());

  assert_int_equal(0, allocate_area(280, 20)); // [280, 299]

  // there are still two memory areas
  assert_int_equal(2, free_memory_area_index);
  assert_int_equal(2, memory_areas_size());

  assert_int_equal(0, free_memory_areas[0].start);
  assert_int_equal(100, free_memory_areas[0].size);

  assert_int_equal(200, free_memory_areas[1].start);
  assert_int_equal(80, free_memory_areas[1].size);
}

// Used area aligns exactly with the start of a free area --> makes the free
// area smaller at the start
void test_allocate_area_completely_overlaps_free_area() {

  clear_placement_memory_array();

  // there is no free memory area
  assert_int_equal(0, free_memory_area_index);
  assert_int_equal(0, memory_areas_size());

  // insert a 100 byte block at the start
  assert_int_equal(0, insert_area(0, 100));
  assert_int_equal(0, insert_area(200, 100));

  // there are two free memory areas
  assert_int_equal(2, free_memory_area_index);
  assert_int_equal(2, memory_areas_size());

  assert_int_equal(0, allocate_area(0, 100));

  // there are still two memory areas
  assert_int_equal(1, free_memory_area_index);
  assert_int_equal(1, memory_areas_size());

  assert_int_equal(200, free_memory_areas[0].start);
  assert_int_equal(100, free_memory_areas[0].size);
}

void test_allocate_area_split_free_area_no_space_left() {

  clear_placement_memory_array();

  // dump_free_memory_map();

  // there is no free memory area
  assert_int_equal(0, free_memory_area_index);
  assert_int_equal(0, memory_areas_size());

  // dump_free_memory_map();

  // insert a 1000 byte block at the start
  assert_int_equal(0, insert_area(0, 100));
  assert_int_equal(1, free_memory_area_index);

  // dump_free_memory_map();

  assert_int_equal(0, insert_area(1000, 10));
  assert_int_equal(2, free_memory_area_index);

  // dump_free_memory_map();

  assert_int_equal(0, insert_area(2000, 10));
  assert_int_equal(3, free_memory_area_index);

  // dump_free_memory_map();

  assert_int_equal(0, insert_area(3000, 10));
  assert_int_equal(4, free_memory_area_index);

  // dump_free_memory_map();

  assert_int_equal(0, insert_area(4000, 10));
  assert_int_equal(5, free_memory_area_index);

  // dump_free_memory_map();

  assert_int_equal(0, insert_area(5000, 10));
  assert_int_equal(6, free_memory_area_index);

  // dump_free_memory_map();

  assert_int_equal(0, insert_area(6000, 10));
  assert_int_equal(7, free_memory_area_index);

  // dump_free_memory_map();

  assert_int_equal(0, insert_area(7000, 10));
  assert_int_equal(8, free_memory_area_index);

  // dump_free_memory_map();

  assert_int_equal(0, insert_area(8000, 10));
  assert_int_equal(9, free_memory_area_index);

  // dump_free_memory_map();

  // printf("insert 9000\n");

  assert_int_equal(0, insert_area(9000, 10));
  assert_int_equal(10, free_memory_area_index);

  // dump_free_memory_map();

  // printf("free_memory_area_index: %d\n", free_memory_area_index);

  // printf("insert 10000\n");

  assert_int_equal(-1, insert_area(10000, 10));
  assert_int_equal(10, free_memory_area_index);
  assert_int_equal(10, memory_areas_size());

  assert_int_equal(-4, allocate_area(50, 10));
}

void test_allocate_area_split_free_area_space_left() {

  clear_placement_memory_array();

  // dump_free_memory_map();

  // there is no free memory area
  assert_int_equal(0, free_memory_area_index);
  assert_int_equal(0, memory_areas_size());

  // dump_free_memory_map();

  // insert a 1000 byte block at the start
  assert_int_equal(0, insert_area(0, 100));
  assert_int_equal(1, free_memory_area_index);

  // dump_free_memory_map();

  assert_int_equal(0, insert_area(1000, 10));
  assert_int_equal(2, free_memory_area_index);

  // dump_free_memory_map();

  assert_int_equal(0, insert_area(2000, 10));
  assert_int_equal(3, free_memory_area_index);

  // dump_free_memory_map();

  assert_int_equal(0, insert_area(3000, 10));
  assert_int_equal(4, free_memory_area_index);

  // dump_free_memory_map();

  assert_int_equal(0, insert_area(4000, 10));
  assert_int_equal(5, free_memory_area_index);

  // dump_free_memory_map();

  assert_int_equal(0, insert_area(5000, 10));
  assert_int_equal(6, free_memory_area_index);

  // dump_free_memory_map();

  assert_int_equal(0, insert_area(6000, 10));
  assert_int_equal(7, free_memory_area_index);

  // dump_free_memory_map();

  assert_int_equal(0, insert_area(7000, 10));
  assert_int_equal(8, free_memory_area_index);

  // dump_free_memory_map();

  assert_int_equal(0, insert_area(8000, 10));
  assert_int_equal(9, free_memory_area_index);

  // dump_free_memory_map();

  // printf("insert 9000\n");

  // assert_int_equal(0, insert_area(9000, 10));
  // assert_int_equal(10, free_memory_area_index);

  // dump_free_memory_map();

  // printf("free_memory_area_index: %d\n", free_memory_area_index);

  // printf("insert 10000\n");

  // assert_int_equal(-1, insert_area(10000, 10));
  // assert_int_equal(10, free_memory_area_index);
  // assert_int_equal(10, memory_areas_size());

  // dump_free_memory_map();
  assert_int_equal(0, allocate_area(50, 10));
  // dump_free_memory_map();

  assert_int_equal(10, free_memory_area_index);
  assert_int_equal(10, memory_areas_size());

  assert_int_equal(0, free_memory_areas[0].start);
  assert_int_equal(50, free_memory_areas[0].size);

  assert_int_equal(60, free_memory_areas[1].start);
  assert_int_equal(40, free_memory_areas[1].size);

  assert_int_equal(1000, free_memory_areas[2].start);
  assert_int_equal(10, free_memory_areas[2].size);
}

void test_allocate_area_split_free_area() {

  clear_placement_memory_array();

  // there is no free memory area
  assert_int_equal(0, free_memory_area_index);
  assert_int_equal(0, memory_areas_size());

  // insert a 100 byte block at the start
  assert_int_equal(0, insert_area(0, 100));
  assert_int_equal(0, insert_area(200, 100));

  // there is only a single memory area
  assert_int_equal(2, free_memory_area_index);
  assert_int_equal(2, memory_areas_size());

  assert_int_equal(-3, allocate_area(500, 100));
}

void test_random_data() {

  clear_placement_memory_array();

  // there is no free memory area
  assert_int_equal(0, free_memory_area_index);
  assert_int_equal(0, memory_areas_size());

  // starting from 1MB up to 128 MB
  assert_int_equal(0, insert_area(1048576, 134086655));

  // dump_free_memory_map();

  // allocate 38 byte
  assert_int_equal(0, allocate_area(1073152, 38));

  // dump_free_memory_map();
}

void test_dump() {

  clear_placement_memory_array();

  // there is no memory area
  assert_int_equal(0, free_memory_area_index);
  assert_int_equal(0, memory_areas_size());

  // insert a 100 byte block at the start
  assert_int_equal(0, insert_area(0, 100));

  // insert a kilobyte
  assert_int_equal(0, insert_area(200, 1024));

  // insert a 1.5 megabyte
  assert_int_equal(0, insert_area(2000, 1024 * 1024 * 1.5f));

  // insert a 1.5 gigabyte
  assert_int_equal(0,
                   insert_area(1024 * 1024 * 2.0f, 1024 * 1024 * 1024 * 1.5f));

  // k_print_float(123.456f);

  dump_free_memory_map();
}

// clang-format off
/*

make clean
rm ./testmain
gcc -c sort.c random.c placement_memory.c
gcc -c placement_memory_test.c -I/home/wbi/dev/cmocka-1.1.5/include
gcc -L /home/wbi/dev/cmocka-1.1.5-build/src -o "testmain" ./placement_memory_test.o ./sort.o ./random.o ./placement_memory.o -lcmocka
./testmain

*/
// clang-format on
int main(int argc, char **argv) {

  // clang-format off
  const UnitTest tests[] = {

      unit_test(test_contains_A), 
      unit_test(test_contains_B),
      unit_test(test_contains_itself), unit_test(test_overlapp),
      unit_test(test_clear_placement_memory_array),
      unit_test(test_kalloc_none_free),
      unit_test(test_kalloc_zero),
      unit_test(test_kalloc_all_free),
      unit_test(test_kalloc_consume_all),
      unit_test(test_two_allocations),
      unit_test(test_kalloc_not_enough_memory),
      unit_test(test_kalloc_too_small),
      unit_test(test_kalloc_second_area_serves),
      unit_test(test_insert_into_empty),
      unit_test(test_insert_area_already_used),
      unit_test(test_insert_area_already_used_successor_collision),
      unit_test(test_insert_area_already_used_predecessor_collision),
      unit_test(test_insert_area_already_used_last_collision),
      unit_test(test_insert_merge_successor),
      unit_test(test_insert_merge_predecessor),
      unit_test(test_insert_merge_both_sides),
      unit_test(test_allocate_area_no_free_areas),
      unit_test(test_allocate_area_size_zero),
      unit_test(test_allocate_area_no_overlap_with_free_area),
      unit_test(test_allocate_area_overlaps_with_start_free_area),
      unit_test(test_allocate_area_overlaps_with_end_free_area),
      unit_test(test_allocate_area_completely_overlaps_free_area),
      unit_test(test_allocate_area_split_free_area_no_space_left),
      unit_test(test_allocate_area_split_free_area_space_left),

      unit_test(test_random_data),

      //unit_test(test_dump),
  };
  // clang-format on

  return run_tests(tests);
}