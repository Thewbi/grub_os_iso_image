#include "random.h"

unsigned long retrieve_pseudo_random(int x0) {

  // TODO: fix this: look up a pseudo random number generator on the internet.
  // return retrieve_rdrand();

  // https://wiki.osdev.org/Random_Number_Generator - Linear congruential
  // generator

  unsigned long m = 0x100000000; // 2 ^ 32, pow(2, 32)
  unsigned long a = 1103515245;
  unsigned long c = 12345;

  return (a * x0 + c) % m;
}

/*
This function uses the RDRAND CPU instruction to return a random number using
CPU entropy. Check if RDRAND is available first, using is_rdrand_available().
*/
unsigned int retrieve_rdrand() {

  unsigned int random_value = 0;

  asm("rdrand %%eax; \        
        movl %%eax, %0;"
      : "=r"(random_value) /* output. random_value = %0 */
      :                    /* input */
      : "%eax"             /* clobbered register */
  );

  return random_value;
}

/*
This function returns true if the CPU supports the RDRAND operation.

https://wiki.osdev.org/Random_Number_Generator
x86 RDRAND Instruction
Newer x86 and x86-64 processors have the instruction RDRAND for generating
random numbers. To use RDRAND you will first need to check if the instruction is
available.

// clang-format off

mov eax, 1     ; set EAX to request function 1
mov ecx, 0     ; set ECX to request subfunction 0
cpuid
shr ecx, 30    ; the result we want is in ECX...
and ecx, 1     ; ...test for the flag of interest...

// clang-format on
*/
int is_rdrand_available() {

  int rdrand_available = 0;

  // AT&T syntax:
  // mnemonic src, dest

  // clang-format off

  asm("movl $0x01, %%eax; \
        movl $0x00, %%ecx; \
        cpuid; \
        shr $0x1E, %%ecx; \
        and $0x01, %%ecx; \
        movl %%ecx, %0;"
      : "=r"(rdrand_available) /* output. rdrand_available = %0 */
      :                        /* input */
      : "%eax", "%ecx"         /* clobbered register */
  );

  // clang-format on

  return rdrand_available;
}

// https://www.ibiblio.org/gferg/ldp/GCC-Inline-Assembly-HOWTO.html
void asmtest() {

  int x = 5;
  int five_times_x = 0;

  // clang-format off

  asm("leal (%1,%1,4), %0" 
      : "=r"(five_times_x) 
      : "r" (x)
      );

  // clang-format on

  printf("x = %d\n", x);
  printf("five_times_x = %d\n", five_times_x);
}