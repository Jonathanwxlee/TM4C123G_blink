
.syntax unified
.cpu cortex-m4
.thumb

.global g_pfnVectors
.global Reset_Handler
.global Default_Handler

.extern main
.extern _estack
.extern _sidata
.extern _sdata
.extern _edata
.extern _sbss
.extern _ebss

.section .isr_vector, "a", %progbits
.balign 128

g_pfnVectors:
    .word _estack
    .word Reset_Handler

    // Core exception handlers
    .rept 14
    .word Default_Handler
    .endr

.section .text.Reset_Handler
.thumb_func

Reset_Handler:

    // Copy initialized data from Flash to RAM
    ldr r0, =_sidata
    ldr r1, =_sdata
    ldr r2, =_edata

copy_data:
    cmp r1, r2
    bcs zero_bss

    ldr r3, [r0], #4
    str r3, [r1], #4
    b copy_data

    // Zero-initialize the BSS section
zero_bss:
    ldr r1, =_sbss
    ldr r2, =_ebss
    movs r3, #0

zero_loop:
    cmp r1, r2
    bcs call_main

    str r3, [r1], #4
    b zero_loop

call_main:
    bl main

hang:
    b hang

.thumb_func
Default_Handler:
    b Default_Handler
