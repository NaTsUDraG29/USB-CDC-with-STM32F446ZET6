/* ============================================================================
 * Linker Script for STM32F446ZET6 (BigTreeTech Octopus)
 * Architecture: ARM Cortex-M4F
 * Flash: 512 KB (0x08000000 - 0x0807FFFF)
 * SRAM:  128 KB (0x20000000 - 0x2001FFFF)
 * ============================================================================ */

/* Entry Point: Where the debugger/CPU starts execution */
ENTRY(Reset_Handler)

/* Highest address of the user stack (Top of RAM, full descending stack) */
_estack = ORIGIN(RAM) + LENGTH(RAM); /* 0x20000000 + 128K = 0x20020000 */

/* Specify the memory regions available on the STM32F446ZET6 */
MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 512K
    RAM   (xrw) : ORIGIN = 0x20000000, LENGTH = 128K
}

SECTIONS
{
    /* 1. Vector Table: MUST sit at the very beginning of Flash (0x08000000) */
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector)) /* Keep vector table even with link-time garbage collection */
        . = ALIGN(4);
    } > FLASH

    /* 2. Program Code and constants */
    .text :
    {
        . = ALIGN(4);
        *(.text)           /* Program code (.text) */
        *(.text*)          /* Subroutines (.text.function_name) */
        *(.glue_7)         /* Glue arm to thumb code */
        *(.glue_7t)        /* Glue thumb to arm code */
        *(.eh_frame)

        KEEP (*(.init))
        KEEP (*(.fini))

        . = ALIGN(4);
        _etext = .;        /* End of code in Flash */
    } > FLASH

    /* 3. Read-Only Data (strings, const variables) */
    .rodata :
    {
        . = ALIGN(4);
        *(.rodata)         /* Read-only data */
        *(.rodata*)        /* Read-only data sub-sections */
        . = ALIGN(4);
    } > FLASH

    /* C++ exception unwinding (discard or hold) */
    .ARM.extab : { *(.ARM.extab* .gnu.linkonce.armextab.*) } > FLASH
    .ARM : {
        __exidx_start = .;
        *(.ARM.exidx*)
        __exidx_end = .;
    } > FLASH

    /* 4. Initialized Data:
     *    Stored in FLASH (Load Memory Address - LMA)
     *    Executed from RAM (Virtual Memory Address - VMA)
     *    Startup code must copy this from _sidata to _sdata.
     */
    _sidata = LOADADDR(.data);

    .data :
    {
        . = ALIGN(4);
        _sdata = .;        /* Start of .data in RAM */
        *(.data)           /* Initialized data */
        *(.data*)          /* Initialized data sub-sections */
        . = ALIGN(4);
        _edata = .;        /* End of .data in RAM */
    } > RAM AT> FLASH

    /* 5. Uninitialized Data (BSS) and Common Symbols:
     *    Resides in RAM, takes NO space in Flash.
     *    Startup code must zero-fill from _sbss to _ebss.
     */
    .bss :
    {
        . = ALIGN(4);
        _sbss = .;         /* Start of .bss in RAM */
        __bss_start__ = _sbss;
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;         /* End of .bss in RAM */
        __bss_end__ = _ebss;
    } > RAM

    /* Discard debugging info from standard C libraries if not needed */
    /DISCARD/ :
    {
        libc.a ( * )
        libm.a ( * )
        libgcc.a ( * )
    }
}
