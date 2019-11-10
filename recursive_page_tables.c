#include "recursive_page_tables.h"

// 1. retrieve 4kB = 4096 bytes from placemenet memory
// HAS TO BE 4k ALIGNED!

// 2. create page directory there
// last entry points to physical start address of the page directory

// 3. Create page tables for the first 15 MB of identity mapped memory

// ?. retrieve another 4kB = 4096 bytes from placemenet memory
// create a page table. Map that page table to the virtual address at 2 GB

void initialize_page_directory() {}