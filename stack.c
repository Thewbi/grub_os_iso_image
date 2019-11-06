#include "stack.h"

// Because C cannot function without a stack, the stack has to be set up in
// assembly. Please take a look at boot.asm for the stack setup code.

// This file only marks the bytes used by the stack allocated in the placement
// memory map so the stack is not overidden.

// According to https://wiki.osdev.org/Bare_Bones
// in the System V ABI, the stack has to be 16-byte aligned on x86.