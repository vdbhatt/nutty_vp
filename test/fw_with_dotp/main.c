
#define VECTOR_A_REG =   0x80080000
#define VECTOR_B_REG =   0x80080004
#define VECTOR_LENGTH =  0x80080008
#define VECTOR_COMMAND = 0x8008000c

void simple_isr() {
    // end of mac read out the mac value and finish the execution
    // by this time trace should already create number of memory access
    asm volatile ("li a2, 0x8008000c" );
    asm volatile ("li a3, 0x00000010" );
    asm volatile ("sw a3, 0(a2)" );
    // for(;;);
    __asm__("ebreak");
}

int main(){
    asm volatile("la t0, CONTEXT_SWITCH");
    asm volatile("csrw mtvec, t0");
    asm volatile("li t1, 0x888"); // mie = 1; mpie = 1 ; mpp = 1
    asm volatile("csrw mie, t1");

    // enable interrupts
    asm volatile( "csrs mstatus,0x0F" );


    // load data into memory
    asm volatile ("li a2, 0x80000000" );
    asm volatile ("li a3, 1" );
    asm volatile ("sw a3, 0(a2)" );
    asm volatile ("li a3, 2" );
    asm volatile ("sw a3, 4(a2)" );
    asm volatile ("li a3, 3" );
    asm volatile ("sw a3, 8(a2)" );
    asm volatile ("li a3, 4" );
    asm volatile ("sw a3, 12(a2)" );
    // load b vector
    asm volatile ("li a3, 1" );
    asm volatile ("sw a3, 16(a2)" );
    asm volatile ("li a3, 2" );
    asm volatile ("sw a3, 20(a2)" );
    asm volatile ("li a3, 3" );
    asm volatile ("sw a3, 24(a2)" );
    asm volatile ("li a3, 4" );
    asm volatile ("sw a3, 28(a2)" );
    // laod a register
    asm volatile ("li a2, 0x80080000" );
    asm volatile ("li a3, 0x80000000" );
    asm volatile ("sw a3, 0(a2)" );
    // load b regiter;
    asm volatile ("li a2, 0x80080004" );
    asm volatile ("li a3, 0x80000010" );
    asm volatile ("sw a3, 0(a2)" );
    // load the destination register
    asm volatile ("li a2, 0x80080010" );
    asm volatile ("li a3, 0x80000020" );
    asm volatile ("sw a3, 0(a2)" );
    // load the length
    asm volatile ("li a2, 0x80080008" );
    asm volatile ("li a3, 0x00000004" );
    asm volatile ("sw a3, 0(a2)" );

    // trigger the start
    asm volatile ("li a2, 0x8008000c" );
    asm volatile ("li a3, 0x00000001" );
    asm volatile ("sw a3, 0(a2)" );
    for(;;);
    return 0;
}