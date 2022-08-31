#pragma once

#define IMM_TYPE 1
#define MEM_SIZE 0xFFFFF
#define SP MEM_SIZE - 4
#define BASE_ADDR 0x80000000
// Table 2.4: Currently allocated RISC-V machine-level CSR addresses.
// machine information registers
#define mvendorid 0xf11
#define marchid 0xf12
#define mimpid 0xf13
#define mhartid 0xf14
// mahcine trap setup
#define mtrap_setup_base 0x300
#define mstatus (mtrap_setup_base + 0x0)
#define misa (mtrap_setup_base + 0x1)
#define medeleg 0x302
#define mideleg 0x303
#define mie 0x304
#define mstatus_mie (1 << 3)
#define mip_meip (1 << 11)
#define mtvec 0x305
#define mcounteren 0x306
// machine trap handling
#define mscratch 0x340
#define mepc 0x341
#define mcause 0x342
#define mtval 0x343
#define mip 0x344
// machine memory protection
// forget for now ;)

#define sstatus 0x100
#define sie 0x104
#define stvec 0x105
#define sepc 0x141