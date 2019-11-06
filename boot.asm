bits 32

section .multiboot               ; according to multiboot spec
        dd 0x1BADB002            ; set magic number for bootloader
        dd 0x0                   ; set flags
        dd - (0x1BADB002 + 0x0)  ; set checksum

section .text
global start
extern main                      ; defined in the C file

start:
        cli                      ; block interrupts
        mov esp, stack_space     ; set stack pointer
        
        push   ebx               ; Push the pointer to the Multiboot information structure.
        push   eax               ; Push the magic value.

        call main                ; call main
        hlt                      ; halt the CPU

section .bss
resb 8192                        ; 8KB for stack at the start of the bss section
stack_space:                     ; this label points to the upper bound of the area reserved for the stack
                                 ; this upper bound is loaded into esp a few lines above (mov esp, stack_space).