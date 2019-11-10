#include "bitmap.h"
#include "descriptor_tables.h"
#include "isr.h"
#include "memory_map.h"
#include "multiboot.h"
#include "placement_memory.h"
#include "recursive_page_tables.h"

#define RESERVED_MEMORY_IN_BYTES 10 * 1024 * 1024
#define SIZE_OF_FRAME_IN_BYTES 4 * 1024

// end is defined in the linker script.
extern unsigned int end;

extern multiboot_uint64_t placement_memory;

extern uint32_t bitmap[BITMAP_ELEMENT_COUNT];

unsigned int allocate_frame_aligned(multiboot_uint64_t *placement_memory,
                                    unsigned int size_in_byte);

long factorial(int n) {
  if (n == 0)
    return 1;
  else
    return (n * factorial(n - 1));
}

void switch_page_directory(page_directory_t *dir);

void page_fault_interrupt_handler(registers_t regs);

void paging_test(multiboot_uint64_t placement_memory_address);

//
//
//
void processELF(multiboot_info_t *mbi) {

  // k_printf("Reading ELF information ...\n");

  // Is the section header table of ELF valid?
  if (!CHECK_FLAG(mbi->flags, 5)) {

    // k_printf("No ELF information found.\n");
    return;
  }

  elf_section_header_table_t *elf_sec = &(mbi->u.elf_sec);

  // k_printf("elf_sec: num = %d, size = 0x%x,"
  //          " addr = 0x%x, shndx = 0x%x\n",
  //          elf_sec->num, elf_sec->size, elf_sec->addr, elf_sec->shndx);

  // block memory
  int address = allocate_area(elf_sec->addr, elf_sec->size);

  // if we do not tell the multibootloader how large our text and bss segments
  // are via the multiboot header, it will not mark the areas of the memory as
  // used! The elf size is not accurate if we do not fully and correctly set all
  // fields in the multiboot header! Since the fields have not been set, the elf
  // data is not valid. assume a large kernel:
  //
  // Assume the kernel takes up 10 MB of space
  // int address = allocate_area(elf_sec->addr, 1024 * 1024 * 10);

  // Do not care about allocation errors. If there is an error, the elf was
  // loaded to blocked memory already which is fine. if (address < 0) {
  //   k_printf("Allocating memory failed!");
  //   return;
  // }

  // k_printf("Reading ELF information done.\n");
}

//
//
//
void processMods(multiboot_info_t *mbi) {

  // k_printf("Reading multiboot modules ...\n");

  // Are mods_* valid?
  if (!CHECK_FLAG(mbi->flags, 3)) {

    // k_printf("No multiboot modules found!\n");
    return;
  }

  module_t *mod;
  int i;
  int j;

  // k_printf("mods_count = %d, mods_addr = 0x%x\n", mbi->mods_count,
  //          mbi->mods_addr);

  for (i = 0, mod = (module_t *)mbi->mods_addr; i < mbi->mods_count;
       i++, mod += sizeof(module_t)) {

    // k_printf("Module %d) mod_start = 0x%x, mod_end = 0x%x, string = %s\n",
    //          i + 1, mod->mod_start, mod->mod_end, (char *)mod->string);

    // k_dump_free_memory_map();

    // block memory
    int address = allocate_area(mod->mod_start, mod->mod_end - mod->mod_start);

    // DO not care about failed allocation at this point
    // if (address < 0) {
    //   k_printf("Allocating memory failed!");
    //   return;
    // }

    /*
          // DEBUG: output the first characters from the test module
          char *character = mod->mod_start;
          for (j = 0; j < 37; j++) {
            // putchar(&mod->mod_start);
            putchar((*character));
            character++;
          }
          printf("\n");
          */

    /*
          memory_area_t rhs;
          rhs.start = mod->mod_start;
          rhs.size = mod->mod_end - mod->mod_start;

          // DEBUG: check if it is contained in a free memory area
          for (int i = 0; i < 10; i++) {
            if (contains(&free_memory_areas[i], &rhs)) {
              printf("contained in free memory area.\n");
            }
          }

          // DEBUG: check if it is contained in a used memory area
          for (int i = 0; i < 10; i++) {
            if (contains(&used_memory_areas[i], &rhs)) {
              printf("contained in used memory area .\n");
            }
          }
    */
  }

  // k_printf("Reading multiboot modules done!\n");
}

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
    // // MB. By allocating the first 639.0 KB, the entire first MB is reserved.
    // int address = allocate_area(0, 639 * 1024);
    // if (address < 0) {
    //   k_printf("Allocating memory failed!\n");
    //   return;
    // }

    // find where the kenel elf file was loaded and exclude it from free
    // memory
    processELF(mbi);

    // subtract all loaded modules from free memory!
    processMods(mbi);

    // k_dump_free_memory_map();
  }

  // TODO: setup the stack and then exclude the stack from free memory
  // The stack is already set up in stack.asm

  int esp_address = 0;
  int ebp_address = 0;

  // AT&T syntax:
  // mnemonic src, dest

  // clang-format off

  // load esp
  __asm__ __volatile__("movl %%esp, %0;"
      : "=r"(esp_address) // output
      :                   // input
      :                   // clobbered register  
  );

  // load ebp
  __asm__ __volatile__("movl %%ebp, %0;"
      : "=r"(ebp_address) // output
      :                   // input
      :                   // clobbered register  
  );

  // clang-format on

  // k_printf("ESP: %d\n", esp_address);
  // k_printf("EBP: %d\n", ebp_address);

  // the stack pointers are in the middle of free memory!!! although the
  // boot.asm file sets up the stack to use memory in the kernels .bss section!
  // If the ELF multiboot code would be valid, that stack memory should have
  // been allocated in processELF() because processELF()'s job is to reserve all
  // the memory used by the kernel file! Obviously the processELF() code is
  // flawed and I do not understand how to make sense of the multiboot
  // information.

  // allocate_area(esp_address, ebp_address - esp_address);

  // just block the first ten MB of RAM because I do not understand the output
  // of the multiboot information enough to figure out where the kernel really
  // uses memory!
  allocate_area_cover(0, RESERVED_MEMORY_IN_BYTES);

  // k_dump_free_memory_map();

  // setup placement_memory pointer and allocate that area
  // placement_memory = free_memory_areas[0].start;
  multiboot_uint64_t placement_memory_address =
      allocate(PLACEMENT_MEMORY_SIZE_IN_BYTES);
  if (placement_memory_address == 0) {

    k_printf("Setting up placement memory failed!\n");

    return;
  }

  // identity map all frames from 0 up to the start

  // of the first free entry in the placement memory table
  // mark the first 15 MB of ram used in the bitmap of frames
  reset_bitmap(bitmap, BITMAP_ELEMENT_COUNT);
  if (use_frames_in_mb(bitmap, BITMAP_ELEMENT_COUNT,
                       RESERVED_MEMORY_IN_BYTES +
                           PLACEMENT_MEMORY_SIZE_IN_BYTES)) {

    k_printf("Setting up the bitmap failed!\n");

    return;
  }

  k_printf("Paging Setup ...\n");

  k_printf("Reset page directory ...\n");

  // uint32_t page_directory[1024] __attribute__((aligned(4096)));
  // uint32_t page_directory[1024];

  // 1 page directory
  // 4 page tables for 16 MB placement memory
  // 1024 entries each
  // 4 byte = 32 bit per entry
  uint32_t *paging_ptr = 0;
  paging_ptr =
      allocate_frame_aligned(&placement_memory_address, ((4 + 1) * 1024 * 4));

  k_printf("paging_ptr: %d\n", paging_ptr);

  uint32_t *page_directory_ptr = 0;
  page_directory_ptr = paging_ptr;

  k_printf("page_directory_ptr: %d\n", page_directory_ptr);

  // set each entry to not present
  int i;
  for (i = 0; i < 1024; i++) {

    // This sets the following flags to the pages:
    //   * Supervisor: Only kernel-mode can access them
    //   (If the bit is set,
    //   then the page may be accessed by all;
    //   if the bit is not set,however,
    //   only the supervisor can access it.)
    //
    //   * Write Enabled: It can be both read from
    //   and written to
    //
    //   * Not Present: The page table is not present
    page_directory_ptr[i] = 0x00000002;
  }

  k_printf("Reset page directory done.\n");

  k_printf("Applying recursive trick to page directory ...\n");

  // last entry stores address of the table
  page_directory_ptr[1023] = page_directory_ptr;

  k_printf("page_directory_ptr[1023]: %d\n", page_directory_ptr[1023]);

  k_printf("Applying recursive trick to page directory done.\n");

  k_printf("Initializing page tables ...\n");

  unsigned int page_table_count = 4;
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

  // we will fill all 1024*page_table_count entries in each
  // page table, mapping 4*page_table_count megabytes
  for (unsigned int i = 0; i < 1024 * page_table_count; i++) {

    // As the address is page aligned,
    // it will always leave 12 bits zeroed.

    // Those bits are used by the attributes
    // attributes: supervisor level, read/write,present.
    page_tables_ptr[i] = (i * 0x1000) | 3;
  }

  k_printf("Initializing page tables done.\n");

  k_printf("Applying recursive trick to page tables ...\n");

  uint32_t *tempA = page_tables_ptr;
  for (unsigned int i = 0; i < page_table_count; i++) {

    // last entry stores address of the table
    page_tables_ptr[i * 1024 + 1023] = tempA;
    tempA += 1024;

    k_printf("page_tables_ptr[1023]: %d\n", page_tables_ptr[i * 1024 + 1023]);
  }

  k_printf("Applying recursive trick to page tables done.\n");

  k_printf("Inserting page tables int page directory ...\n");

  // put page tables into the page directory
  uint32_t *temp = page_tables_ptr;
  for (unsigned int i = 0; i < page_table_count; i++) {

    // attributes: supervisor level, read/write, present
    page_directory_ptr[i] = temp;
    page_directory_ptr[i] |= 3;

    // increment to next page_table
    k_printf("temp: %d\n", temp);
    k_printf("page_directory_ptr: %d\n", page_directory_ptr[i]);
    temp += 1024;

    // k_printf("Address of page table: %d\n", page_directory_ptr[i]);
  }

  k_printf("Inserting page tables int page directory done.\n");

  /**/

  k_printf("Setting CR3 and CR0 ...\n");

  // put page directory into cr3
  // clang-format off

  __asm__ __volatile__("mov %0, %%cr3;"
      :                           // output
      : "r"(page_directory_ptr)   // input
  );

  // clang-format on

  k_printf("register page_fault_interrupt_handler ...\n");

  // Before we enable paging, we must register our page fault handler.
  register_interrupt_handler(14, page_fault_interrupt_handler);

  k_printf("register page_fault_interrupt_handler done.\n");

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

  k_printf("Setting CR3 and CR0 done.\n");

  k_printf("Paging Setup done.\n");

  // k_dump_free_memory_map();

  unsigned long fact = factorial(10);
  k_printf("Factorial of 10: %d\n", fact);

  u32int *ptr = 0;
  u32int do_page_fault = 0;

  ptr = (u32int *)0x00000000;
  do_page_fault = *ptr;

  // put a memory on a area of memory that is unmapped
  ptr = (u32int *)0xA0000000;

  // access that area which then causes a page fault
  do_page_fault = *ptr;

  // after main returns, program flow jumps back to boot.asm which executes hlt
  // to hlt the CPU
  return;
}

void paging_test(multiboot_uint64_t placement_memory_address) {

  // allocate placement memory for the page directory
  // this will return the 4k aligned physical address of memory
  // to construct the page directory in

  unsigned int frames_per_page_directory = 1;
  unsigned int frames_per_page_table = 1;
  unsigned int size_managed_by_page_table_in_MB = 4;

  unsigned int reserved_in_MB =
      RESERVED_MEMORY_IN_BYTES + PLACEMENT_MEMORY_SIZE_IN_BYTES;
  reserved_in_MB /= 1024 * 1024;

  unsigned int page_tables_required = 0;
  while (page_tables_required * size_managed_by_page_table_in_MB <
         reserved_in_MB) {
    page_tables_required++;
  }

  // k_printf("page_tables_required: %d\n", page_tables_required);

  unsigned int frames_required =
      page_tables_required * frames_per_page_table + frames_per_page_directory;

  // k_printf("Before: placement_memory_address: %d\n",
  // placement_memory_address);

  // ptr now as enough space to house a page directory and
  // four page tables to cover the entire placement memory
  unsigned char *ptr = 0;
  ptr = allocate_frame_aligned(&placement_memory_address,
                               (frames_required * 4096));

  // k_printf("After: placement_memory_address: %d\n",
  // placement_memory_address);

  k_printf("ptr: %d\n", ptr);

  // the page directory
  // page_directory_t *ptr_page_directory = 0;
  // ptr_page_directory = (page_directory_t *)ptr;

  page_table_t *ptr_page_directory = 0;
  ptr_page_directory = (page_table_t *)ptr;

  // set each entry to not present
  int i;
  for (i = 0; i < 1024; i++) {

    // reset
    ptr_page_directory->pages[i].accessed = 0;
    ptr_page_directory->pages[i].dirty = 0;
    ptr_page_directory->pages[i].frame = 0;
    ptr_page_directory->pages[i].present = 0;
    ptr_page_directory->pages[i].rw = 0;
    ptr_page_directory->pages[i].unused = 0;
    ptr_page_directory->pages[i].user = 0;
  }

  // the last entry in the page directory contains a physical pointer to itself
  // ptr_page_directory->tables[1023] = ptr_page_directory;

  page_table_t *page_table_0 = ptr + 4096 * 1;
  page_t *page = &(page_table_0->pages[1023]);
  *((unsigned int *)page) = page_table_0;
  k_printf("recurse 0: %d\n", page_table_0->pages[1023]);
  k_printf("page_table_0: %d\n", page_table_0);

  // connect directory to page table
  //*(ptr_page_directory->pages[0]) = *page_table_0;
  ptr_page_directory->pages[0].frame = ((u32int)(page_table_0)) >> 12;
  ptr_page_directory->pages[0].present = 1;

  page_table_t *page_table_1 = ptr + 4096 * 2;
  page = &(page_table_1->pages[1023]);
  *((unsigned int *)page) = page_table_1;
  k_printf("recurse 1: %d\n", page_table_1->pages[1023]);
  k_printf("page_table_1: %d\n", page_table_1);

  // connect directory to page table
  //*(ptr_page_directory->tables[1]) = *page_table_1;
  ptr_page_directory->pages[1].frame = ((u32int)(page_table_1)) >> 12;
  ptr_page_directory->pages[1].present = 1;

  page_table_t *page_table_2 = ptr + 4096 * 3;
  page = &(page_table_2->pages[1023]);
  *((unsigned int *)page) = page_table_2;
  k_printf("recurse 2: %d\n", page_table_2->pages[1023]);
  k_printf("page_table_2: %d\n", page_table_2);

  // connect directory to page table
  //*(ptr_page_directory->tables[2]) = *page_table_2;
  ptr_page_directory->pages[2].frame = ((u32int)(page_table_2)) >> 12;
  ptr_page_directory->pages[2].present = 1;

  page_table_t *page_table_3 = ptr + 4096 * 4;
  page = &(page_table_3->pages[1023]);
  *((unsigned int *)page) = page_table_3;
  k_printf("recurse 3: %d\n", page_table_3->pages[1023]);
  k_printf("page_table_3: %d\n", page_table_3);

  // connect directory to page table
  //*(ptr_page_directory->tables[3]) = *page_table_3;
  ptr_page_directory->pages[3].frame = ((u32int)(page_table_3)) >> 12;
  ptr_page_directory->pages[3].present = 1;

  // setup first page table (1024 entries, but the last entry is a recursive
  // pointer to physical memory, so only 1024 - 1 entries are connected to
  // frames. )

  unsigned int frame_index = 0;

  initialize_page_table(&frame_index, page_table_0);
  initialize_page_table(&frame_index, page_table_1);
  initialize_page_table(&frame_index, page_table_2);
  initialize_page_table(&frame_index, page_table_3);

  // Before we enable paging, we must register our page fault handler.
  register_interrupt_handler(14, page_fault_interrupt_handler);

  // Now, enable paging!
  switch_page_directory(ptr_page_directory);

  // Test 1: write a certain value into placement memory
  // Enable paging and identity map all frames from 0 - to used memory
  // With paging enabled, read the value which is supposed to be available
  // because it was identity mapped. make sure the value is read correctly.

  // Test 2: read the text module
  // The text module is identity mapped and with paging enabled should be
  // available under the same address

  // Test 3: the qemu virtual machine has only 200 MB of RAM
  // with paging enabled, allocate a page at 1000MB of RAM (virtual address) and
  // write a value into that RAM location. Read the value back and compare. The
  // algorithm has to be able to add page directory entries and page table
  // entries for that memory location aven after paging has been enabled. It has
  // to map the page to a phyisically available frame in the phyiscal RAM 0-200
  // MB.
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

void initialize_page_table(unsigned int *frame_index,
                           page_table_t *page_table) {

  // for (int i = 0; i < 1023; i++) {
  for (int i = 0; i < 1024; i++) {

    // reset
    page_table->pages[i].accessed = 0;
    page_table->pages[i].dirty = 0;
    page_table->pages[i].frame = 0;
    page_table->pages[i].present = 0;
    page_table->pages[i].rw = 0;
    page_table->pages[i].unused = 0;
    page_table->pages[i].user = 0;

    // set values
    page_table->pages[i].frame = (*frame_index) * 0x1000;
    page_table->pages[i].present = 1; // Mark it as present.
    page_table->pages[i].rw = 1;      // Should the page be writeable?

    (*frame_index)++;
  }

  // skip frame for physical recursion pointer
  //(*frame_index)++;
}

void switch_page_directory(page_directory_t *dir) {

  // current_directory = dir;

  //__asm__ __volatile__("mov %0, %%cr3" ::"r"(&dir->tablesPhysical));
  //__asm__ __volatile__("mov %0, %%cr3" ::"r"(&dir->tables[1023]));

  // clang-format off

  __asm__ __volatile__("mov %0, %%cr3;" 
      :                           // output
      : "r"(dir)   // input
  );

  // clang-format on

  u32int cr0;

  // asm volatile("mov %%cr0, %0" : "=r"(cr0));

  // AT&T syntax:
  // mnemonic src, dest

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
}

void page_fault_interrupt_handler(registers_t regs) {

  // A page fault has occurred.
  // The faulting address is stored in the CR2 register.
  u32int faulting_address;
  __asm__ __volatile__("mov %%cr2, %0" : "=r"(faulting_address));

  // The error code gives us details of what happened.
  int present = !(regs.err_code & 0x1); // Page not present
  int rw = regs.err_code & 0x2;         // Write operation?
  int us = regs.err_code & 0x4;         // Processor was in user-mode?
  int reserved =
      regs.err_code & 0x8;       // Overwritten CPU-reserved bits of page entry?
  int id = regs.err_code & 0x10; // Caused by an instruction fetch?

  // k_printf("page fault !!!");

  // Output an error message.
  k_printf("Page fault! ( ");
  if (present) {
    k_printf("present ");
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

  // this goes into an endless loop.
  // the endless loop is started because otherwise the CPU would just jump
  // back and execute the code that caused the page fault again, which then
  // causes the page fault again, as currently the interrupt handler does
  // not fix the problem and an endless loop starts.
  //
  // Calling panic will just output a message onto the scree once and then
  // start the endless loop so the user has time to read the error message
  // on the screen
  PANIC("Page fault");
}
