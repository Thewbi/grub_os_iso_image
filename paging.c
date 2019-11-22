#include "paging.h"

uint32_t *heap_start = (uint32_t *)(50 * 1024 * 1024);
uint32_t *heap_end = (uint32_t *)(50 * 1024 * 1024);

int brk(void *addr) {

  heap_end = addr;

  return 0;
}

void *sbrk(int incr) {

  uint32_t *result = heap_end;

  heap_end += incr;

  return (void *)result;
}

/**
 * @brief adding a page to the paging table and directory
 *
 * This is called in the page_fault_interrupt_handler()
 *
 * @param virtual_address
 */
void setup_page(uint32_t virtual_address) {

  // k_printf("virtual_address: %x\n", virtual_address);

  // find next free frame to house a page table
  int page_table_frame_index =
      next_free_frame_index(bitmap, BITMAP_ELEMENT_COUNT);

  if (page_table_frame_index < 0) {

    k_printf("No free frames left!\n");

    return;

  } else {

    // k_printf("First free frame index: %d\n", page_table_frame_index);

    // set frame used
    set(bitmap, BITMAP_ELEMENT_COUNT, page_table_frame_index, 1);

    // compute physical address of the frame for the page table
    uint32_t *page_table_frame_address_physical =
        (uint32_t *)(page_table_frame_index * 0x1000);

    // compute the index inside the page directory for the virtual_address
    uint32_t page_directory_index = virtual_address / (4 * 1024 * 1024);
    // k_printf("page_directory_index: %d\n", page_directory_index);

    // compute the index inside the page table for the virtual_address
    uint32_t page_table_index =
        (virtual_address % (4 * 1024 * 1024)) / (4 * 1024);
    // k_printf("page_table_index: %d\n", page_table_index);

    // compute the page table address using the recursive trick
    u32int *page_table_ptr =
        (u32int *)(0xFFC00000 + page_directory_index * 0x1000);

    // k_printf("Retrieving page directory pointer ...\n");

    // this pointer lets us write into the page directory at the
    // page directory entry that houses the new page table
    u32int *page_directory_ptr =
        (u32int *)(0xFFFFF000 + page_directory_index * 4);

    // k_printf("Retrieving page directory pointer done.!\n");

    // k_printf("Installing page directory entry ...\n");

    if ((*page_directory_ptr & 1) == 0) {

      // k_printf("Page directory ...\n");

      // clear it
      *page_directory_ptr = 0;

      // put the physical address to the frame in that houses the page directory
      *page_directory_ptr = (uint32_t)page_table_frame_address_physical;

      // mark it present
      *page_directory_ptr |= 3;

      // only initilaze the page table if it was just created
      // and do not initialize it every time the method is called
      // k_printf("Initializing page table ...\n");
      initialize_page_table(page_table_ptr, page_table_frame_address_physical);
      // k_printf("Initializing page table done.\n");
    }

    // k_printf("Installing page directory entry done.\n");

    // k_printf("Dumping page directory via the recursive trick ...\n");

    // access the page directory while paging is on:
    //
    // 0xFFFFF000 is a virtual address that is managed by the last entry of
    // the page directory. The first ten bits of 0xFFFFF000 are set to one, so
    // it is managed by the last entry.
    // Into this last entry, we wrote the physical address
    // of where the page directory is placed in physical memory. So now the
    // MMU is looking at the physical page table. The address 0xFFFFF000 also
    // is taken care of by the last entry.
    // Usually an entry in the page directory points to the address of a page
    // table. Not in this special case! In this special case the physical
    // address points to the page directory again instead of to some page table!
    // The MMU now tries to resolve the next 10 bit in the address. The next
    // 10 bit are set to 1 again, so that address points to the last entry in
    // the page directory again! The last entry again points to the physical
    // address of the page directory. The MMU performed the same loop twice and
    // now points to the page directory.
    //
    // Compute the magic recursive address to
    // the page table N.B: The page table is indexed via the
    // page_directory_index, not the page_table_index!!!! page_directory_index
    // is an index inside the page directory see
    // https://medium.com/@connorstack/recursive-page-tables-ad1e03b20a85
    u32int *recursive_page_directory_pointer = (u32int *)0xFFFFF000;

    // dump_table(recursive_page_directory_pointer);

    // k_printf("Dumping page directory via the recursive trick done.\n");

    // k_printf("Creating data frame ...\n");

    // find another free frame to put behind the page table entry and mark it
    // used. This frame will actually store that data and the page table entry
    // will point to it
    int data_frame_index = next_free_frame_index(bitmap, BITMAP_ELEMENT_COUNT);

    if (data_frame_index < 0) {

      // k_printf("No free frames left for the data frame!\n");

      return;
    }

    // k_printf("Second free frame index: %d\n", data_frame_index);

    // mark the data frames used
    set(bitmap, BITMAP_ELEMENT_COUNT, data_frame_index, 1);

    // k_printf("Creating data frame done.\n");

    // k_printf("Connecting data frame to page table ...\n");

    // multiply the frame's index in the bitmap with the size of a frame to
    // compute a free physical memory address of that particular frame
    uint32_t *data_frame_address_physical =
        (uint32_t *)(data_frame_index * 0x1000);

    // set the address to the data frame into the page table entry
    page_table_ptr[page_table_index] = (uint32_t)data_frame_address_physical;

    // make the page table entry present
    page_table_ptr[page_table_index] |= 1;

    // k_printf("Connecting data frame to page table done.\n");

    // k_printf("Dumping page table via the recursive trick...\n");

    // compute the magic recursive address to the page table
    // N.B: The page table is indexed via the page_directory_index,
    // not the page_table_index!!!! page_directory_index is an index inside
    // the page directory
    // see https://medium.com/@connorstack/recursive-page-tables-ad1e03b20a85

    // u32int *recursive_page_table_pointer =
    //    (u32int *)(0xFFC00000 + (page_directory_index * 0x1000));

    // dump_table(recursive_page_table_pointer);

    // k_printf("Dumping page table via the recursive trick done.\n");
  }
}

void initialize_page_table(u32int *page_table_ptr,
                           uint32_t *page_table_frame_address_physical) {

  // here the page table is accessed before the recursive trick is applied
  // which fortunately works without any issues ...

  // create a page table
  // first initialize it and set each entry to "not present"
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
    //   * Present: The page table is not present because
    //   the last bit (P) is 0
    page_table_ptr[i] = 0x00000002;
  }

  // recursive trick for page table
  page_table_ptr[1023] = (uint32_t)page_table_frame_address_physical;
}

void dump_table(u32int *table_ptr) {

  for (int i = 0; i < 1023; i++) {

    uint32_t page_table_entry = table_ptr[i];

    // bit 1 is set if the page entry is present in memory
    if (page_table_entry & 1) {

      k_printf("Entry %d is present!\n", i);
    }
  }
}

void page_fault_interrupt_handler(registers_t regs) {

  // A page fault has occurred.
  // The faulting address is stored in the CR2 register.
  u32int faulting_address;
  __asm__ __volatile__("mov %%cr2, %0" : "=r"(faulting_address));

  // The error code gives us details of what happened.

  // Page not present
  int present = regs.err_code & 0x1;

  // Write operation?
  int rw = regs.err_code & 0x2;

  // Processor was in user-mode?
  int us = regs.err_code & 0x4;

  // Overwritten CPU-reserved bits of page entry?
  int reserved = regs.err_code & 0x8;

  // Caused by an instruction fetch?
  int id = regs.err_code & 0x10;

  // k_printf("page fault !!!");

  // Output an error message.
  k_printf("Page fault! ( ");

  if (!present) {
    k_printf("not present ");
  }

  if (rw) {
    k_printf("read-only ");
  }

  if (us) {
    k_printf("user-mode ");
  }

  if (reserved) {
    k_printf("reserved ");
  }

  k_printf(") at 0x%x", faulting_address);
  k_printf("\n");

  // create a page at the location that the application wants to access
  setup_page(faulting_address);
}