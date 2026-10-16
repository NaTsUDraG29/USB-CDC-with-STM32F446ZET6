/* ============================================================================
 * STM32F446ZET6 (BigTreeTech Octopus) Stage-1 Bootloader
 * ============================================================================
 * Flash Partition: 0x08000000 - 0x08007FFF (32 KB)
 * Application Partition: 0x08008000 - 0x0807FFFF (480 KB)
 * ============================================================================ */

#include <stdint.h>
#include "stm32f446.h"

#define APP_START_ADDRESS   0x08008000UL

static void delay_ms(volatile uint32_t ms)
{
    while (ms--) {
        for (volatile uint32_t i = 0; i < 2000; i++) {
            __asm volatile ("nop");
        }
    }
}

static void usart_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIODEN;
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    for (volatile int i = 0; i < 100; i++);

    /* PA9 (TX1), PA10 (RX1) -> TFT Header */
    GPIOA->MODER &= ~((0x3UL << (9 * 2)) | (0x3UL << (10 * 2)));
    GPIOA->MODER |=  ((0x2UL << (9 * 2)) | (0x2UL << (10 * 2)));
    GPIOA->OSPEEDR |= ((0x2UL << (9 * 2)) | (0x2UL << (10 * 2)));
    GPIOA->PUPDR &= ~((0x3UL << (9 * 2)) | (0x3UL << (10 * 2)));
    GPIOA->PUPDR |=  (0x1UL << (10 * 2));
    GPIOA->AFR[1] &= ~((0xFUL << 4) | (0xFUL << 8));
    GPIOA->AFR[1] |=  ((0x7UL << 4) | (0x7UL << 8));

    /* PD5 (TX2), PD6 (RX2) -> RPi Header */
    GPIOD->MODER &= ~((0x3UL << (5 * 2)) | (0x3UL << (6 * 2)));
    GPIOD->MODER |=  ((0x2UL << (5 * 2)) | (0x2UL << (6 * 2)));
    GPIOD->OSPEEDR |= ((0x2UL << (5 * 2)) | (0x2UL << (6 * 2)));
    GPIOD->PUPDR &= ~((0x3UL << (5 * 2)) | (0x3UL << (6 * 2)));
    GPIOD->PUPDR |=  (0x1UL << (6 * 2));
    GPIOD->AFR[0] &= ~((0xFUL << 20) | (0xFUL << 24));
    GPIOD->AFR[0] |=  ((0x7UL << 20) | (0x7UL << 24));

    USART1->BRR = 0x008B; /* 115200 @ 16 MHz HSI */
    USART1->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;

    USART2->BRR = 0x008B; /* 115200 @ 16 MHz HSI */
    USART2->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
}

static void uart_putc(char c)
{
    if (c == '\n') {
        uart_putc('\r');
    }
    while (!(USART1->SR & USART_SR_TXE));
    USART1->DR = (uint8_t)c;
    while (!(USART2->SR & USART_SR_TXE));
    USART2->DR = (uint8_t)c;
}

static void uart_puts(const char *str)
{
    while (*str) {
        uart_putc(*str++);
    }
}

static void uart_puthex(uint32_t val)
{
    uart_puts("0x");
    for (int i = 7; i >= 0; i--) {
        uint8_t nibble = (val >> (i * 4)) & 0x0F;
        char c = (nibble < 10) ? ('0' + nibble) : ('A' + nibble - 10);
        uart_putc(c);
    }
}

/* ============================================================================
 * Jump to Application Function
 * ============================================================================ */
static void jump_to_application(uint32_t app_sp, uint32_t app_entry)
{
    uart_puts("[BOOTLOADER] Disabling interrupts and prepping CPU registers...\r\n");
    uart_puts("[BOOTLOADER] Jumping to Application now!\r\n");
    uart_puts("--------------------------------------------------------------\r\n\r\n");

    /* Ensure UART finishes sending all bytes before shutting down */
    while (!(USART1->SR & USART_SR_TC));
    while (!(USART2->SR & USART_SR_TC));

    /* 1. Disable all interrupts */
    __asm volatile ("cpsid i");

    /* 2. Disable the SysTick timer completely */
    *(volatile uint32_t *)0xE000E010UL = 0; /* SysTick CTRL */
    *(volatile uint32_t *)0xE000E014UL = 0; /* SysTick LOAD */
    *(volatile uint32_t *)0xE000E018UL = 0; /* SysTick VAL */

    /* 2. De-initialize peripherals back to reset defaults */
    USART1->CR1 = 0;
    USART2->CR1 = 0;

    /* 3. Relocate Vector Table in Cortex-M System Control Block */
    SCB->VTOR = APP_START_ADDRESS;

    /* 4. Set Main Stack Pointer (MSP) to Application's initial stack pointer */
    __asm volatile ("msr msp, %0" : : "r" (app_sp) : );

    /* 5. Branch to Application's Reset_Handler */
    void (*app_reset_handler)(void) = (void (*)(void))app_entry;
    app_reset_handler();
}

int main(void)
{
    usart_init();

    uart_puts("\r\n");
    uart_puts("==============================================================\r\n");
    uart_puts("     BIGTREETECH OCTOPUS STM32F446 BOOTLOADER (STAGE 1)       \r\n");
    uart_puts("==============================================================\r\n");
    uart_puts(" Bootloader Origin : 0x08000000 (32 KB)\r\n");
    uart_puts(" Application Target: "); uart_puthex(APP_START_ADDRESS); uart_puts(" (480 KB)\r\n");
    uart_puts(" Inspecting vector table at "); uart_puthex(APP_START_ADDRESS); uart_puts("...\r\n");

    /* Read first two words of Application vector table */
    uint32_t app_sp    = *(volatile uint32_t *)(APP_START_ADDRESS + 0);
    uint32_t app_entry = *(volatile uint32_t *)(APP_START_ADDRESS + 4);

    uart_puts(" -> Application Initial SP    : "); uart_puthex(app_sp);    uart_puts("\r\n");
    uart_puts(" -> Application Reset Handler : "); uart_puthex(app_entry); uart_puts("\r\n");

    /* Validate stack pointer: must be within SRAM (0x20000000 - 0x20020000) */
    if ((app_sp & 0xFFFC0000) == 0x20000000 && (app_entry & 0x08000000) == 0x08000000) {
        uart_puts("[BOOTLOADER] Valid Application detected!\r\n");
        uart_puts("[BOOTLOADER] Booting Application in 1.5 seconds...\r\n");
        delay_ms(1500);

        jump_to_application(app_sp, app_entry);
    } else {
        uart_puts("\r\n[BOOTLOADER] [!] WARNING: No valid application image at ");
        uart_puthex(APP_START_ADDRESS);
        uart_puts("!\r\n");
        uart_puts("[BOOTLOADER] Flash the application binary using:\r\n");
        uart_puts("             make flash-app   (or STM32_Programmer_CLI -w app.bin 0x08008000)\r\n");
        uart_puts("[BOOTLOADER] Staying in bootloader mode.\r\n\r\n");
    }

    /* Fallback loop if no application is flashed */
    uint32_t count = 0;
    while (1) {
        delay_ms(1000);
        count++;
        uart_puts("[BOOTLOADER WAITING #");
        uart_puthex(count);
        uart_puts("] Ready for flash at 0x08008000.\r\n");
    }

    return 0;
}
