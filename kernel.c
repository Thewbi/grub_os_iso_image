#include "bitmap.h"
#include "descriptor_tables.h"
#include "heap.h"
#include "isr.h"
#include "memory_map.h"
#include "multiboot.h"
#include "paging.h"
#include "pci.h"
#include "placement_memory.h"
#include "recursive_page_tables.h"

#define RESERVED_MEMORY_IN_BYTES 40 * 1024 * 1024
#define SIZE_OF_FRAME_IN_BYTES 4 * 1024

unsigned int page_table_count = (40 / 4);

// end is defined in the linker script.
extern unsigned int end;

extern multiboot_uint64_t placement_memory;

multiboot_uint64_t placement_memory_address = 0;

extern uint32_t bitmap[BITMAP_ELEMENT_COUNT];

unsigned int allocate_frame_aligned(multiboot_uint64_t *placement_memory,
                                    unsigned int size_in_byte);

long factorial(int n);

void paging_setup();

void paging_tests();

void main(unsigned long magic, unsigned long addr) {

  // Set MBI to the address of the Multiboot information structure.
  multiboot_info_t *mbi = (multiboot_info_t *)addr;

  // Initialise all the ISRs and segmentation
  init_descriptor_tables();

  clear_placement_memory_array();

  terminal_buffer = (unsigned short *)VGA_ADDRESS;

  // clear_screen();
  k_cls();
  vga_index = 0;

  // use the address of end! Not its value!
  // printf("End from linker script: 0x%x\n", &end);

  // Am I booted by a Multiboot-compliant boot loader?
  if (magic == MULTIBOOT_BOOTLOADER_MAGIC) {

    // printf("Booted by a multiboot bootloader magic number!\n");

    if (process_multiboot_memory_map(mbi)) {

      k_printf("Processing placement memory failed!\n");

      return;
    }

    // k_printf("\n");

    // // subtract the first Megabyte from free memory as it is used by basic
    // // BIOS stuff.
    // // The first 639.0 KB are free, then everything is reserved up the the
    // first
    // // MB. By allocating the first 639.0 KB, the entire first MB is
    // reserved. int address = allocate_area(0, 639 * 1024); if (address < 0)
    // {
    //   k_printf("Allocating memory failed!\n");
    //   return;
    // }

    // find where the kenel elf file was loaded and exclude it from free
    // memory
    // processELF(mbi);

    // subtract all loaded modules from free memory!
    // processMods(mbi);

    // k_dump_free_memory_map();
  }

  // TODO: setup the stack and then exclude the stack from free memory
  // The stack is already set up in stack.asm

  // just block the first ten MB of RAM because I do not understand the output
  // of the multiboot information enough to figure out where the kernel really
  // uses memory!
  allocate_area_cover(0, RESERVED_MEMORY_IN_BYTES);

  // k_dump_free_memory_map();

  // setup placement_memory pointer and allocate that area
  // placement_memory = free_memory_areas[0].start;
  placement_memory_address = allocate(PLACEMENT_MEMORY_SIZE_IN_BYTES);
  if (placement_memory_address == 0) {

    k_printf("Setting up placement memory failed!\n");

    return;
  }

  // reset the bitmap of used frames
  reset_bitmap(bitmap, BITMAP_ELEMENT_COUNT);

  // identity map all frames from 0 up to the start

  // of the first free entry in the placement memory table
  // mark the first ? MB of RAM used in the bitmap of frames
  if (use_frames_in_bytes(bitmap, BITMAP_ELEMENT_COUNT,
                          RESERVED_MEMORY_IN_BYTES +
                              PLACEMENT_MEMORY_SIZE_IN_BYTES)) {

    k_printf("Setting up the bitmap failed!\n");

    return;
  }

  // k_printf("First element: %d\n", bitmap[0]);
  // k_printf("%d\n", next_free_frame_index(bitmap, BITMAP_ELEMENT_COUNT));

  // identity maps ? Megabyte then turns paging on
  paging_setup(page_table_count);

  paging_tests();

  // k_dump_free_memory_map();

  // // pci
  // k_printf("PCI ...\n");
  // init_pcilist();

  void *mem_ptr = k_malloc((size_t)1000);
  k_free(mem_ptr);
  mem_ptr = NULL;

  unsigned long fact = factorial(10);
  k_printf("Factorial of 10: %d\n", fact);

  // after main returns, program flow jumps back to boot.asm which executes
  // hlt to hlt the CPU
  return;
}

unsigned int allocate_frame_aligned(multiboot_uint64_t *placement_memory,
                                    unsigned int size_in_byte) {

  // increment placement_memory_address to a 4k boundary.
  // If the address is not already page-aligned
  if ((*placement_memory & 0xFFFFF000)) {

    // Align it.

    // decrement to 4k boundary
    *placement_memory &= 0xFFFFF000;

    // increment it to a free boundary
    *placement_memory += 0x1000;
  }

  k_printf("Aligned: placement_memory_address: %d\n", *placement_memory);

  k_printf("Aligned: size_in_byte: %d\n", size_in_byte);

  // allocate the amount of bytes
  unsigned int tmp = *placement_memory;
  k_printf("tmp %d\n", tmp);

  *placement_memory += size_in_byte;

  return tmp;
}

void paging_setup(int page_table_count) {

  // k_printf("Paging Setup ...\n");

  // k_printf("Reset page directory ...\n");

  // uint32_t page_directory[1024] __attribute__((aligned(4096)));
  // uint32_t page_directory[1024];

  // 1 page directory
  // 4 page tables for 16 MB placement memory
  // 1024 entries each
  // 4 byte = 32 bit per entry
  unsigned int page_directory_count = 1;
  unsigned int size_in_byte =
      ((page_directory_count + page_table_count) * 1024 * 4);
  uint32_t *paging_ptr = (uint32_t *)allocate_frame_aligned(
      &placement_memory_address, size_in_byte);

  // k_printf("paging_ptr: %d\n", paging_ptr);
  uint32_t *page_directory_ptr = paging_ptr;

  // k_printf("page_directory_ptr: %d\n", page_directory_ptr);

  // set each entry in the page directory to not present
  for (int i = 0; i < 1024; i++) {

    // This sets the following flags to the pages:
    //
    //   * Supervisor: Only kernel-mode can access them
    //   (If the bit is set,
    //   then the page may be accessed by all;
    //   if the bit is not set,however,
    //   only the supervisor can access it.)
    //
    //   * Write Enabled: It can be both read from
    //   and written to
    //
    //   * Present: The page table is not present because the last bit (P) is
    //   0
    page_directory_ptr[i] = 0x00000002;
  }

  // k_printf("Reset page directory done.\n");

  // k_printf("Applying recursive trick to page directory ...\n");

  // last entry stores address of the table
  page_directory_ptr[1023] = (uint32_t)page_directory_ptr;

  // set the present flag otherwise the MMU will signal a page fault
  page_directory_ptr[1023] |= 1;

  // k_printf("page_directory_ptr[1023]: %d\n", page_directory_ptr[1023]);

  // k_printf("Applying recursive trick to page directory done.\n");

  // k_printf("Initializing page tables ...\n");

  // unsigned int page_table_count = 4;
  // uint32_t page_tables[1024 * page_table_count]
  // __attribute__((aligned(4096)));

  // set the page_tables pointer right after the page directory
  uint32_t *page_tables_ptr = paging_ptr;
  // k_printf("page_tables_ptr: %d\n", page_tables_ptr);
  page_tables_ptr += 1024;
  // k_printf("page_tables_ptr: %d\n", page_tables_ptr);

  // this line causes the OS to crash! I do not know why.
  // uint32_t first_page_table[1024] __attribute__((aligned(4096)));

  // holds the physical address where we want to start mapping these
  // pages to. in this case, we want to map these pages to the very
  // beginning of memory.

  // we will fill all 1024 * page_table_count entries in each
  // page table, mapping 4*page_table_count megabytes
  for (unsigned int i = 0; i < 1024 * page_table_count; i++) {

    // As the address is page aligned,
    // it will always leave 12 bits zeroed.

    // Those bits are used by the attributes
    // attributes: supervisor level, read/write,present.
    page_tables_ptr[i] = (i * 0x1000) | 3;
  }

  // k_printf("Initializing page tables done.\n");

  // k_printf("Applying recursive trick to page tables ...\n");

  uint32_t *tempA = page_tables_ptr;
  for (unsigned int i = 0; i < page_table_count; i++) {

    // last entry stores address of the table
    page_tables_ptr[i * 1024 + 1023] = (uint32_t)tempA;

    // the Present bit has to be set, otherwise the MMU will signal a page
    // fault
    page_tables_ptr[i * 1024 + 1023] |= 1;

    // increment the address by sizeof(uint32_t) * 1024 = 4 * 1024 = 4K bytes
    tempA += 1024;

    // k_printf("page_tables_ptr[1023]: %d\n", page_tables_ptr[i * 1024 +
    // 1023]);
  }

  // k_printf("Applying recursive trick to page tables done.\n");

  // k_printf("Inserting page tables int page directory ...\n");

  // put page tables into the page directory
  uint32_t *temp = page_tables_ptr;
  for (unsigned int i = 0; i < page_table_count; i++) {

    // attributes: supervisor level, read/write, present
    page_directory_ptr[i] = (uint32_t)temp;
    page_directory_ptr[i] |= 3;

    // increment to next page_table
    // k_printf("temp: %d\n", temp);
    // k_printf("page_directory_ptr: %d\n", page_directory_ptr[i]);
    temp += 1024;

    // k_printf("Address of page table: %d\n", page_directory_ptr[i]);
  }

  // k_printf("Inserting page tables int page directory done.\n");

  // k_printf("register page_fault_interrupt_handler ...\n");

  // Before we enable paging, we must register our page fault handler.
  register_interrupt_handler(14, page_fault_interrupt_handler);

  // k_printf("register page_fault_interrupt_handler done.\n");

  // k_printf("Setting CR3 and CR0 ...\n");

  // put page directory into cr3
  // clang-format off

  __asm__ __volatile__("mov %0, %%cr3;"
      :                           // output
      : "r"(page_directory_ptr)   // input
  );

  // clang-format on

  // We must set the 32th bit in the CR0 register,
  // the paging bit. This operation also requires
  // assembly code. Once done, paging will be enabled.
  //
  // The strategy is to read cr0, alter it to flip the bit,
  // then writeit back to preserve most of its state except
  // the paging bit

  // The cr0 register is a 32bit register, so use a 32 bit variable
  u32int cr0;

  // clang-format off

  // copy cr0 content to cr0 variable
  __asm__ __volatile__("mov %%cr0, %0;"
      : "=r"(cr0) // output
      :           // input
      :           // clobbered register
  );

  // clang-format on

  cr0 |= 0x80000000; // Enable paging!

  // asm volatile("mov %0, %%cr0" ::"r"(cr0));

  // clang-format off

  // copy cr0 content to cr0 variable
  __asm__ __volatile__("mov %0, %%cr0;"
      :           // output
      : "r"(cr0)          // input
      :   // clobbered register
  );

  // clang-format on

  // k_printf("Setting CR3 and CR0 done.\n");

  // k_printf("Paging Setup done.\n");
}

void paging_tests() {

  u32int *ptr = (u32int *)0x00000000;
  u32int do_page_fault = *ptr;

  uint32_t virtual_address = 0xA0000000;

  // put a memory on a area of memory that is unmapped
  ptr = (u32int *)virtual_address;

  // access that area which then causes a page fault
  // do_page_fault = *ptr;

  // page_directory_ptr = 0xFFFFF000;

  // k_printf("Setup page ...\n");
  // setup_page(virtual_address);
  // setup_page(virtual_address + 0x1000);
  // k_printf("Setup page done.\n");

  k_printf("PageFault?\n");

  // // access that area which then causes a page fault
  // // put a memory on a area of memory that is unmapped
  // // ptr = (u32int *)0xA0000000;
  // // ptr = (u32int *)0xA0001000;
  // // ptr = (u32int *)0xA0002000;
  ptr = (u32int *)(virtual_address);
  // ptr = (u32int *)(virtual_address + 0x1000);

  // write
  *ptr = 100;

  // read
  do_page_fault = *ptr;

  k_printf("StoredValue: %d\n", do_page_fault);

  ptr = (u32int *)(virtual_address + 0x1000);
  // ptr = (u32int *)(virtual_address + 0x1000);

  // write
  *ptr = 101;

  // read
  do_page_fault = *ptr;

  k_printf("StoredValue: %d\n", do_page_fault);
}

long factorial(int n) {
  if (n == 0)
    return 1;
  else
    return (n * factorial(n - 1));
}