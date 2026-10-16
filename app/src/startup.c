/* ============================================================================
 * Startup Code for STM32F446 Application (Cortex-M4F)
 * Vector Table located at 0x08008000
 * ============================================================================ */

#include <stdint.h>
#include "stm32f4xx_hal.h"

extern uint32_t _estack;
extern uint32_t _sidata;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;

extern int main(void);
extern PCD_HandleTypeDef hpcd;

void Reset_Handler(void);

extern void uart_puts(const char *s);

void Default_Handler(void)
{
    /* If an unexpected interrupt fires, execution traps safely here.
     * In GDB: inspect ICSR register to see active vector:
     * p/x (SCB->ICSR & 0x1FF)
     */
    uart_puts("\r\n[APP TRAP] Unexpected interrupt fired! Check vector table.\r\n");
    while (1);
}

void HardFault_Handler(void)
{
    /* Traps here on HardFault. Inspect SCB->CFSR, HFSR, and ($sp + 24) in GDB */
    uart_puts("\r\n[APP TRAP] HARDFAULT occurred! Trapping in bkpt.\r\n");
    __asm volatile ("bkpt #0");
    while (1);
}

void SysTick_Handler(void)
{
    HAL_IncTick();
}

void OTG_FS_IRQHandler(void)
{
    HAL_PCD_IRQHandler(&hpcd);
}

void NMI_Handler(void)        __attribute__((weak, alias("Default_Handler")));
void MemManage_Handler(void)  __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler(void)   __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void)        __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler(void)   __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void)     __attribute__((weak, alias("Default_Handler")));

/* Vector table: 16 system vectors + 97 external interrupts = 113 total entries */
__attribute__((section(".isr_vector"), used))
const void *vector_table[16 + 97] = {
    [0 ... 16 + 96] = Default_Handler, /* Default all slots to Default_Handler to prevent NULL crashes */
    [0]  = &_estack,
    [1]  = Reset_Handler,
    [2]  = NMI_Handler,
    [3]  = HardFault_Handler,
    [4]  = MemManage_Handler,
    [5]  = BusFault_Handler,
    [6]  = UsageFault_Handler,
    [7]  = 0,
    [8]  = 0,
    [9]  = 0,
    [10] = 0,
    [11] = SVC_Handler,
    [12] = DebugMon_Handler,
    [13] = 0,
    [14] = PendSV_Handler,
    [15] = SysTick_Handler,
    [16 + OTG_FS_IRQn] = OTG_FS_IRQHandler,
};

void Reset_Handler(void)
{
    /* 1. Point Vector Table to Application Address (0x08008000) */
    SCB->VTOR = 0x08008000UL;

    /* 2. Copy .data from Flash to RAM */
    uint32_t *pSrc = &_sidata;
    uint32_t *pDst = &_sdata;
    while (pDst < &_edata) {
        *pDst++ = *pSrc++;
    }

    /* 3. Zero out .bss in RAM */
    pDst = &_sbss;
    while (pDst < &_ebss) {
        *pDst++ = 0;
    }

    /* 4. Enable FPU (CP10 and CP11) */
    SCB->CPACR |= (0xFUL << 20);
    __asm volatile ("dsb");
    __asm volatile ("isb");


    /* 6. Run Application main() */
    main();

    while (1);
}
