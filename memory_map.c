#include "memory_map.h"

int process_multiboot_memory_map(multiboot_info_t *mbi) {

  // k_printf("Reading Memory Map ...\n");

  // Are mmap_* valid?
  if (!CHECK_FLAG(mbi->flags, 6)) {

    // k_printf("No Memory Map found ...\n");
    return -1;
  }

  memory_map_t *mmap;

  // printf("mmap_addr = 0x%x, mmap_length = 0x%x\n", mbi->mmap_addr,
  //       mbi->mmap_length);

  for (mmap = (memory_map_t *)mbi->mmap_addr;
       (unsigned long)mmap < mbi->mmap_addr + mbi->mmap_length;
       mmap = (memory_map_t *)((unsigned long)mmap + mmap->size +
                               sizeof(mmap->size))) {

    // k_printf("base_addr = 0x%x%x, length = 0x%x%x, type = 0x%x\n",
    //          mmap->base_addr_high, mmap->base_addr_low, mmap->length_high,
    //          mmap->length_low, mmap->type);

    // https://www.gnu.org/software/grub/manual/multiboot/multiboot.html#Boot-modules
    //
    // ‘type’ is the variety of address range represented, where a value of 1
    // indicates available RAM, value of 3 indicates usable memory holding
    // ACPI information, value of 4 indicates reserved memory which needs to
    // be preserved on hibernation, value of 5 indicates a memory which is
    // occupied by defective RAM modules and all other values currently
    // indicated a reserved area.

    multiboot_uint64_t start = 0;
    multiboot_uint64_t size = 0;

    switch (mmap->type) {

    case 1:
      // printf("Available RAM\n");

      // free_memory_areas[free_memory_area_index].start =
      //     mmap->base_addr_high * sizeof(long) + mmap->base_addr_low;

      // free_memory_areas[free_memory_area_index].size =
      //     mmap->length_high * sizeof(long) + mmap->length_low;

      // free_memory_area_index++;

      start = mmap->base_addr_high * sizeof(long) + mmap->base_addr_low;
      size = mmap->length_high * sizeof(long) + mmap->length_low;

      if (insert_area(start, size)) {

        // k_printf("Inserting free memory area failed!\n");
        return -2;
      }

      break;

    case 3:
      // printf("Usable memory holding ACPI information\n");
      // used_memory_areas[used_memory_area_index].start =
      //     mmap->base_addr_high * 32 + mmap->base_addr_low;
      // used_memory_areas[used_memory_area_index].size =
      //     mmap->length_high * 32 + mmap->length_low;
      // used_memory_areas_index++;
      break;

    case 4:
      // printf("Reserved memory which needs to be preserved on
      // hibernation\n");
      // used_memory_areas[used_memory_area_index].start =
      //     mmap->base_addr_high * 32 + mmap->base_addr_low;
      // used_memory_areas[used_memory_area_index].size =
      //     mmap->length_high * 32 + mmap->length_low;
      // used_memory_areas_index++;
      break;

    case 5:
      // printf("Defective RAM\n");
      // used_memory_areas[used_memory_area_index].start =
      //     mmap->base_addr_high * 32 + mmap->base_addr_low;
      // used_memory_areas[used_memory_area_index].size =
      //     mmap->length_high * 32 + mmap->length_low;
      // used_memory_areas_index++;
      break;

    default:
      // printf("Reserved Area\n");
      // used_memory_areas[used_memory_area_index].start =
      //     mmap->base_addr_high * 32 + mmap->base_addr_low;
      // used_memory_areas[used_memory_area_index].size =
      //     mmap->length_high * 32 + mmap->length_low;
      // used_memory_areas_index++;
      break;
    }
  }

  // dump_free_memory_map();

  // k_printf("Reading Memory Map done.\n");

  return 0;
}
