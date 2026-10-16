/* ============================================================================
 * STM32F446ZET6 (BigTreeTech Octopus) Bring-Up Bootloader Demo
 * ============================================================================
 * Peripherals used:
 * - USART1: PA9 (TX), PA10 (RX) -> TFT Header
 * - USART2: PD5 (TX), PD6  (RX) -> Raspberry Pi Header
 *
 * Default Clock: 16 MHz HSI (High-Speed Internal RC Oscillator)
 * Baud Rate: 115200 (8N1)
 * ============================================================================ */

#include <stdint.h>
#include "stm32f446.h"

/* Simple busy-wait delay (approximate milliseconds at 16 MHz) */
static void delay_ms(volatile uint32_t ms)
{
    while (ms--) {
        for (volatile uint32_t i = 0; i < 2000; i++) {
            __asm volatile ("nop");
        }
    }
}

/* ============================================================================
 * USART Initialization
 * ============================================================================
 * Clock Tree on Reset:
 * - SYSCLK = 16 MHz (HSI)
 * - AHB Prescaler = 1 -> HCLK = 16 MHz
 * - APB1 Prescaler = 1 -> PCLK1 = 16 MHz (USART2 runs here)
 * - APB2 Prescaler = 1 -> PCLK2 = 16 MHz (USART1 runs here)
 *
 * Baud Rate Calculation for 115200 @ 16 MHz:
 * USARTDIV = f_CK / (16 * BaudRate)
 *          = 16,000,000 / (16 * 115200) = 8.68055
 * Mantissa = 8 (0x08)
 * Fraction = 0.68055 * 16 = 10.888 -> round to 11 (0x0B)
 * BRR = (Mantissa << 4) | Fraction = (8 << 4) | 11 = 0x008B
 * ============================================================================ */
static void usart_init(void)
{
    /* 1. Enable peripheral clocks in RCC */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIODEN;
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    /* Tiny delay for bus clock synchronization */
    for (volatile int i = 0; i < 100; i++);

    /* ------------------------------------------------------------------------
     * Configure USART1 Pins (PA9 = TX, PA10 = RX) -> TFT Header
     * ------------------------------------------------------------------------ */
    /* Set PA9 and PA10 to Alternate Function mode (0b10) */
    GPIOA->MODER &= ~((0x3UL << (9 * 2)) | (0x3UL << (10 * 2)));
    GPIOA->MODER |=  ((0x2UL << (9 * 2)) | (0x2UL << (10 * 2)));

    /* High speed */
    GPIOA->OSPEEDR |= ((0x2UL << (9 * 2)) | (0x2UL << (10 * 2)));

    /* Pull-up on RX */
    GPIOA->PUPDR &= ~((0x3UL << (9 * 2)) | (0x3UL << (10 * 2)));
    GPIOA->PUPDR |=  (0x1UL << (10 * 2));

    /* Map AF7 (USART1) to PA9 and PA10 in AFRH (AFR[1]) */
    /* PA9 is pin index 9 -> (9-8)*4 = bit 4 */
    /* PA10 is pin index 10 -> (10-8)*4 = bit 8 */
    GPIOA->AFR[1] &= ~((0xFUL << 4) | (0xFUL << 8));
    GPIOA->AFR[1] |=  ((0x7UL << 4) | (0x7UL << 8));

    /* ------------------------------------------------------------------------
     * Configure USART2 Pins (PD5 = TX, PD6 = RX) -> Raspberry Pi Header
     * ------------------------------------------------------------------------ */
    /* Set PD5 and PD6 to Alternate Function mode (0b10) */
    GPIOD->MODER &= ~((0x3UL << (5 * 2)) | (0x3UL << (6 * 2)));
    GPIOD->MODER |=  ((0x2UL << (5 * 2)) | (0x2UL << (6 * 2)));

    /* High speed */
    GPIOD->OSPEEDR |= ((0x2UL << (5 * 2)) | (0x2UL << (6 * 2)));

    /* Pull-up on RX */
    GPIOD->PUPDR &= ~((0x3UL << (5 * 2)) | (0x3UL << (6 * 2)));
    GPIOD->PUPDR |=  (0x1UL << (6 * 2));

    /* Map AF7 (USART2) to PD5 and PD6 in AFRL (AFR[0]) */
    /* PD5 is pin index 5 -> 5*4 = bit 20 */
    /* PD6 is pin index 6 -> 6*4 = bit 24 */
    GPIOD->AFR[0] &= ~((0xFUL << 20) | (0xFUL << 24));
    GPIOD->AFR[0] |=  ((0x7UL << 20) | (0x7UL << 24));

    /* ------------------------------------------------------------------------
     * Configure USART Peripherals: 115200 8N1
     * ------------------------------------------------------------------------ */
    USART1->BRR = 0x008B; /* 115200 baud @ 16 MHz */
    USART1->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;

    USART2->BRR = 0x008B; /* 115200 baud @ 16 MHz */
    USART2->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
}

/* Transmit a single character to BOTH USART1 and USART2 */
static void uart_putc(char c)
{
    /* If newline, also transmit carriage return */
    if (c == '\n') {
        uart_putc('\r');
    }

    /* Wait until TXE (Transmit data register empty) flag is set on USART1 */
    while (!(USART1->SR & USART_SR_TXE));
    USART1->DR = (uint8_t)c;

    /* Wait until TXE is set on USART2 */
    while (!(USART2->SR & USART_SR_TXE));
    USART2->DR = (uint8_t)c;
}

/* Transmit a null-terminated string */
static void uart_puts(const char *str)
{
    while (*str) {
        uart_putc(*str++);
    }
}

/* Print a 32-bit unsigned integer in hex */
static void uart_puthex(uint32_t val)
{
    uart_puts("0x");
    for (int i = 7; i >= 0; i--) {
        uint8_t nibble = (val >> (i * 4)) & 0x0F;
        char c = (nibble < 10) ? ('0' + nibble) : ('A' + nibble - 10);
        uart_putc(c);
    }
}

/* Print a 32-bit decimal integer */
static void uart_putdec(uint32_t val)
{
    char buf[11];
    int idx = 0;
    if (val == 0) {
        uart_putc('0');
        return;
    }
    while (val > 0) {
        buf[idx++] = '0' + (val % 10);
        val /= 10;
    }
    while (idx > 0) {
        uart_putc(buf[--idx]);
    }
}

/* Check if a character has been received on either port */
static int uart_getc_nonblocking(char *c)
{
    if (USART1->SR & USART_SR_RXNE) {
        *c = (char)(USART1->DR & 0xFF);
        return 1;
    }
    if (USART2->SR & USART_SR_RXNE) {
        *c = (char)(USART2->DR & 0xFF);
        return 1;
    }
    return 0;
}

/* ============================================================================
 * Main Entry Point
 * ============================================================================ */
int main(void)
{
    usart_init();

    /* Send Welcome Banner */
    uart_puts("\r\n");
    uart_puts("==============================================================\r\n");
    uart_puts("   BIGTREETECH OCTOPUS (STM32F446ZET6) BRING-UP BOOTLOADER   \r\n");
    uart_puts("==============================================================\r\n");
    uart_puts(" Core Architecture : ARM Cortex-M4F\r\n");
    uart_puts(" System Clock      : 16 MHz (Internal HSI Oscillator)\r\n");
    uart_puts(" Flash Origin      : "); uart_puthex(FLASH_BASE); uart_puts(" (512 KB)\r\n");
    uart_puts(" SRAM Origin       : "); uart_puthex(SRAM_BASE);  uart_puts(" (128 KB)\r\n");
    uart_puts(" Active Ports      : USART1 (PA9 - TFT) & USART2 (PD5 - RPi)\r\n");
    uart_puts(" Baud Rate         : 115200 8N1\r\n");
    uart_puts(" Linker & C Runtime: .data copied, .bss zeroed, FPU enabled\r\n");
    uart_puts("==============================================================\r\n");
    uart_puts(" Type any character to test echo, or attach GDB to debug.\r\n\r\n");

    uint32_t heartbeat = 0;
    while (1) {
        char ch;
        /* Check if user sent a byte over serial */
        if (uart_getc_nonblocking(&ch)) {
            uart_puts("[ECHO]: '");
            uart_putc(ch);
            uart_puts("' (ASCII ");
            uart_putdec((uint32_t)ch);
            uart_puts(")\r\n");
        }

        /* Periodic heartbeat message every ~1 second */
        delay_ms(1000);
        heartbeat++;
        uart_puts("[BOOTLOADER HEARTBEAT #");
        uart_putdec(heartbeat);
        uart_puts("] MCU alive, PC/SP intact.\r\n");
    }

    return 0;
}
