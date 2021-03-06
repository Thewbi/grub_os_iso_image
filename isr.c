//
// isr.c -- High level interrupt service routines and interrupt request
// handlers.
//          Part of this code is modified from Bran's kernel development
//          tutorials. Rewritten for JamesM's kernel development tutorials.
//

#include "isr.h"
#include "common.h"
#include "types.h"

isr_t interrupt_handlers[256];

// This gets called from our ASM interrupt handler stub.
void isr_handler(registers_t regs) {

  // if a handler is present
  if (interrupt_handlers[regs.int_no] != 0) {

    // retrieve the handler
    isr_t handler = interrupt_handlers[regs.int_no];

    // call the handler
    handler(regs);

  } else {

    k_printf("I do not understand - isr.c - isr_handler()");
  }
}

void register_interrupt_handler(u8int n, isr_t handler) {
  interrupt_handlers[n] = handler;
}
