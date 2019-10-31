#include "memory_map.h"

void process_multiboot_memory_map(multiboot_info_t *mbi) {

  printf("\nReading Memory Map ...\n");

  // Are mmap_* valid?
  if (!CHECK_FLAG(mbi->flags, 6)) {

    printf("\nNo Memory Map found ...\n");
    return;
  }

  init_placement_memory();

  memory_map_t *mmap;

  // printf("mmap_addr = 0x%x, mmap_length = 0x%x\n", mbi->mmap_addr,
  //       mbi->mmap_length);

  for (mmap = (memory_map_t *)mbi->mmap_addr;
       (unsigned long)mmap < mbi->mmap_addr + mbi->mmap_length;
       mmap = (memory_map_t *)((unsigned long)mmap + mmap->size +
                               sizeof(mmap->size))) {

    printf("base_addr = 0x%x%x, length = 0x%x%x, type = 0x%x\n",
           mmap->base_addr_high, mmap->base_addr_low, mmap->length_high,
           mmap->length_low, mmap->type);

    // https://www.gnu.org/software/grub/manual/multiboot/multiboot.html#Boot-modules
    //
    // ‘type’ is the variety of address range represented, where a value of 1
    // indicates available RAM, value of 3 indicates usable memory holding
    // ACPI information, value of 4 indicates reserved memory which needs to
    // be preserved on hibernation, value of 5 indicates a memory which is
    // occupied by defective RAM modules and all other values currently
    // indicated a reserved area.

    switch (mmap->type) {

    case 1:
      // printf("Available RAM\n");

      free_memory_areas[free_memory_area_index].start =
          mmap->base_addr_high * sizeof(long) + mmap->base_addr_low;

      free_memory_areas[free_memory_area_index].size =
          mmap->length_high * sizeof(long) + mmap->length_low;

      free_memory_area_index++;
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

  printf("Reading Memory Map done.\n");
}

void dump_free_memory_map() {

  printf("Free Memory Areas:\n");

  for (int i = 0; i < free_memory_area_index; i++) {

    // bugged
    printf("Bugged: start = 0x%x size = 0x%x\n", free_memory_areas[i].start,
           free_memory_areas[i].size);

    // fine
    printf("Fine: start = 0x%x", free_memory_areas[i].start);
    printf(" size = 0x%x", free_memory_areas[i].size);
    printf("\n");
  }
}