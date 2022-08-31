OUTPUT_ARCH("riscv")
ENTRY(_start)

MEMORY {
  RAM (rwx) : ORIGIN = 0x80000000, LENGTH = 0xfffff
  ROM (rx) : ORIGIN = 0x00000000, LENGTH = 0xfffff
}

SECTIONS
{
    .text (ORIGIN(ROM)) :
    {
        *(.init)
        *(.text)
        *(.rodata)
        PROVIDE(_etext = .);
    } > ROM

    .data (ORIGIN(RAM)) : ALIGN(4)
    {
        . = ALIGN(4);

        PROVIDE(_global_pointer = . + 0x800);

        PROVIDE(_sdata = .);
        *(.data)
        . = ALIGN(4);
        PROVIDE(_edata = .);
    } > RAM AT> ROM

    .bss (NOLOAD) : ALIGN(4)
    {
         . = ALIGN(4);
        PROVIDE(_sbss = .);
        *(.bss)
        *(COMMON)
         . = ALIGN(4);
        PROVIDE(_ebss = .);
    } > RAM

    /* Stack Pointer - End of Memory */
    PROVIDE(_stack_pointer = ORIGIN(RAM) + LENGTH(RAM));
}