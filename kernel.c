#include "descriptor_tables.h"
#include "memory_map.h"
#include "multiboot.h"
#include "placement_memory.h"

// end is defined in the linker script.
extern unsigned int end;

// long factorial(int n) {
//   if (n == 0)
//     return 1;
//   else
//     return (n * factorial(n - 1));
// }

//
//
//
void processELF(multiboot_info_t *mbi) {

  k_printf("Reading ELF information ...\n");

  // Is the section header table of ELF valid?
  if (!CHECK_FLAG(mbi->flags, 5)) {

    k_printf("No ELF information found.\n");
    return;
  }

  elf_section_header_table_t *elf_sec = &(mbi->u.elf_sec);

  k_printf("elf_sec: num = %d, size = 0x%x,"
           " addr = 0x%x, shndx = 0x%x\n",
           elf_sec->num, elf_sec->size, elf_sec->addr, elf_sec->shndx);

  // block memory
  int address = allocate_area(elf_sec->addr, elf_sec->size);

  // Do not care about allocation errors. If there is an error, the elf was
  // loaded to blocked memory already which is fine. if (address < 0) {
  //   k_printf("Allocating memory failed!");
  //   return;
  // }

  k_printf("Reading ELF information done.\n");
}

//
//
//
void processMods(multiboot_info_t *mbi) {

  k_printf("Reading multiboot modules ...\n");

  // Are mods_* valid?
  if (!CHECK_FLAG(mbi->flags, 3)) {

    // k_printf("No multiboot modules found!\n");
    return;
  }

  module_t *mod;
  int i;
  int j;

  k_printf("mods_count = %d, mods_addr = 0x%x\n", mbi->mods_count,
           mbi->mods_addr);

  for (i = 0, mod = (module_t *)mbi->mods_addr; i < mbi->mods_count;
       i++, mod += sizeof(module_t)) {

    k_printf("Module %d) mod_start = 0x%x, mod_end = 0x%x, string = %s\n",
             i + 1, mod->mod_start, mod->mod_end, (char *)mod->string);

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

  k_printf("Reading multiboot modules done!\n");
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

    // subtract the first Megabyte from free memory as it is used by basic
    // BIOS stuff.
    // The first 639.0 KB are free, then everything is reserved up the the first
    // MB. By allocating the first 639.0 KB, the entire first MB is reserved.
    int address = allocate_area(0, 639 * 1024);
    if (address < 0) {
      k_printf("Allocating memory failed!\n");
      return;
    }

    // find where the kenel elf file was loaded and exclude it from free
    // memory
    processELF(mbi);

    // subtract all loaded modules from free memory!
    processMods(mbi);

    k_dump_free_memory_map();
  }

  // TODO: setup the stack and then exclude the stack from free memory

  // TODO: subtract the placement memory are from free memory!

  // printf("Goodbye World!\n");

  return;
}
