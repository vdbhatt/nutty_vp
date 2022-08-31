.section .init;
.globl _start;
.global CONTEXT_SWITCH
.globl simple_isr

_start:
    la gp, _global_pointer;
    la sp, _stack_pointer;
    j main;