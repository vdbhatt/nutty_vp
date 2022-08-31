.section .init;
.globl _start;
.global CONTEXT_SWITCH
.globl simple_isr

_start:
    la gp, _global_pointer;
    la sp, _stack_pointer;
    j main;

CONTEXT_SWITCH:
/* store execution context on the stack (register content) */
addi    sp, sp, -4 * 32
sw   x1, 0x0(sp)
sw   x4, 3 * 4(sp)
sw   x5, 4 * 4(sp)
sw   x6, 5 * 4(sp)
sw   x7, 6 * 4(sp)
sw   x10, 9 * 4(sp)
sw   x11, 10 * 4(sp)
sw   x12, 11 * 4(sp)
sw   x13, 12 * 4(sp)
sw   x14, 13 * 4(sp)
sw   x15, 14 * 4(sp)
sw   x16, 15 * 4(sp)
sw   x17, 16 * 4(sp)
sw   x28, 27 * 4(sp)
sw   x29, 28 * 4(sp)
sw   x30, 29 * 4(sp)
sw   x31, 30 * 4(sp)

/* load interrupt/trap reason and call external C function to handle it */
csrr    a0, mcause
jal    simple_isr

/* re-store the saved context */
lw    x1, 0x0(sp)
lw    x4, 3 * 4(sp)
lw    x5, 4 * 4(sp)
lw    x6, 5 * 4(sp)
lw    x7, 6 * 4(sp)
lw    x10, 9 * 4(sp)
lw    x11, 10 * 4(sp)
lw    x12, 11 * 4(sp)
lw    x13, 12 * 4(sp)
lw    x14, 13 * 4(sp)
lw    x15, 14 * 4(sp)
lw    x16, 15 * 4(sp)
lw    x17, 16 * 4(sp)
lw    x28, 27 * 4(sp)
lw    x29, 28 * 4(sp)
lw    x30, 29 * 4(sp)
lw    x31, 30 * 4(sp)
addi    sp, sp, 4 * 32
mret