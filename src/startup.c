/* ============================================================================
 * Bare-Metal Startup Code for STM32F446ZET6 (Cortex-M4F)
 * ============================================================================
 * What this file does:
 * 1. Sets up the Cortex-M Vector Table (Stack pointer, Reset, Faults, IRQs).
 * 2. Runs Reset_Handler:
 *    - Copies .data section from FLASH to SRAM
 *    - Zero-fills .bss section in SRAM
 *    - Enables Hardware FPU (CP10 & CP11)
 *    - Calls main()
 * 3. Provides fault handlers for diagnostic debugging in GDB.
 * ============================================================================ */

#include <stdint.h>
#include "stm32f446.h"

/* Linker script exported symbols */
extern uint32_t _estack;   /* End of RAM / Initial Stack Pointer */
extern uint32_t _sidata;   /* Start of .data in Flash (LMA) */
extern uint32_t _sdata;    /* Start of .data in RAM (VMA) */
extern uint32_t _edata;    /* End of .data in RAM */
extern uint32_t _sbss;     /* Start of .bss in RAM */
extern uint32_t _ebss;     /* End of .bss in RAM */

/* Application entry point */
extern int main(void);

/* Prototype for Reset_Handler */
void Reset_Handler(void);

/* Default handler for unimplemented interrupts */
void Default_Handler(void)
{
    while (1) {
        /* Trapped in default handler. Look at active interrupt number in ICSR:
         * In GDB: p/x ((*(uint32_t*)0xE000ED04) & 0x1FF)
         */
    }
}

/* Diagnostic HardFault Handler */
void HardFault_Handler(void)
{
    /* When a HardFault happens, GDB will hit this loop.
     * You can inspect SCB->CFSR, SCB->HFSR, SCB->BFAR to see WHY it faulted.
     * E.g. CFSR & (1<<17) = PRECISERR (Data bus error, e.g. invalid memory access)
     */
    __asm volatile ("bkpt #0"); /* Triggers a breakpoint if GDB is attached */
    while (1);
}

/* Weakly alias standard system exceptions to Default_Handler */
void NMI_Handler(void)        __attribute__((weak, alias("Default_Handler")));
void MemManage_Handler(void)  __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler(void)   __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void)        __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler(void)   __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void)     __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void)    __attribute__((weak, alias("Default_Handler")));

/* Weakly alias peripheral interrupts to Default_Handler */
void USART1_IRQHandler(void)  __attribute__((weak, alias("Default_Handler")));
void USART2_IRQHandler(void)  __attribute__((weak, alias("Default_Handler")));

/* ============================================================================
 * Cortex-M4 Vector Table
 * Placed in the ".isr_vector" section at address 0x08000000
 * ============================================================================ */
__attribute__((section(".isr_vector"), used))
const void *vector_table[] = {
    &_estack,               /* 0x00: Initial Stack Pointer (Top of RAM) */
    Reset_Handler,          /* 0x04: Reset Handler */
    NMI_Handler,            /* 0x08: Non-Maskable Interrupt */
    HardFault_Handler,      /* 0x0C: Hard Fault */
    MemManage_Handler,      /* 0x10: Memory Management Fault */
    BusFault_Handler,       /* 0x14: Bus Fault */
    UsageFault_Handler,     /* 0x18: Usage Fault */
    0, 0, 0, 0,             /* 0x1C-0x28: Reserved */
    SVC_Handler,            /* 0x2C: SVCall */
    DebugMon_Handler,       /* 0x30: Debug Monitor */
    0,                      /* 0x34: Reserved */
    PendSV_Handler,         /* 0x38: PendSV */
    SysTick_Handler,        /* 0x3C: SysTick */

    /* External Device Interrupts (STM32F446 specific subset) */
    [16 + 37] = USART1_IRQHandler, /* IRQ 37: USART1 */
    [16 + 38] = USART2_IRQHandler, /* IRQ 38: USART2 */
};

/* ============================================================================
 * Reset Handler: Executed directly on power-up or hardware reset
 * ============================================================================ */
void Reset_Handler(void)
{
    /* 1. Copy initialized data (.data) from Flash (LMA) to RAM (VMA) */
    uint32_t *pSrc = &_sidata;
    uint32_t *pDst = &_sdata;
    while (pDst < &_edata) {
        *pDst++ = *pSrc++;
    }

    /* 2. Zero-initialize the .bss section in RAM */
    pDst = &_sbss;
    while (pDst < &_ebss) {
        *pDst++ = 0;
    }

    /* 3. Enable Cortex-M4 Hardware FPU (Full access to CP10 & CP11)
     * CPACR is at 0xE000ED88. Bits [23:20] must be set to 0b1111 (0xF)
     */
    CPACR_ADDR |= (0xFUL << 20);
    __asm volatile ("dsb");
    __asm volatile ("isb");

    /* 4. Call application main() */
    main();

    /* 5. Trap if main ever exits */
    while (1);
}

