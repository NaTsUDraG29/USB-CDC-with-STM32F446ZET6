#ifndef STM32F446_H
#define STM32F446_H

#include <stdint.h>

/* ============================================================================
 * Memory Base Addresses
 * ============================================================================ */
#define FLASH_BASE          0x08000000UL
#define SRAM_BASE           0x20000000UL /* 128 KB SRAM total (112KB SRAM1 + 16KB SRAM2) */
#define PERIPH_BASE         0x40000000UL

#define APB1PERIPH_BASE     PERIPH_BASE
#define APB2PERIPH_BASE     (PERIPH_BASE + 0x00010000UL)
#define AHB1PERIPH_BASE     (PERIPH_BASE + 0x00020000UL)

/* Peripherals */
#define USART2_BASE         (APB1PERIPH_BASE + 0x4400UL)
#define USART1_BASE         (APB2PERIPH_BASE + 0x1000UL)

#define GPIOA_BASE          (AHB1PERIPH_BASE + 0x0000UL)
#define GPIOB_BASE          (AHB1PERIPH_BASE + 0x0400UL)
#define GPIOC_BASE          (AHB1PERIPH_BASE + 0x0800UL)
#define GPIOD_BASE          (AHB1PERIPH_BASE + 0x0C00UL)
#define RCC_BASE            (AHB1PERIPH_BASE + 0x3800UL)

/* ARM Cortex-M4 Core System Control Space */
#define SCS_BASE            0xE000E000UL
#define SCB_BASE            (SCS_BASE + 0x0D00UL)
#define CPACR_ADDR          (*(volatile uint32_t *)(SCS_BASE + 0x0D88UL)) /* Coprocessor Access Control */

/* ============================================================================
 * Peripheral Register Structures
 * ============================================================================ */
typedef struct {
    volatile uint32_t MODER;    /* 0x00: Mode register (00=in, 01=out, 10=AF, 11=analog) */
    volatile uint32_t OTYPER;   /* 0x04: Output type register (0=push-pull, 1=open-drain) */
    volatile uint32_t OSPEEDR;  /* 0x08: Output speed register */
    volatile uint32_t PUPDR;    /* 0x0C: Pull-up/pull-down register */
    volatile uint32_t IDR;      /* 0x10: Input data register */
    volatile uint32_t ODR;      /* 0x14: Output data register */
    volatile uint32_t BSRR;     /* 0x18: Bit set/reset register */
    volatile uint32_t LCKR;     /* 0x1C: Configuration lock register */
    volatile uint32_t AFR[2];   /* 0x20: Alternate function registers (AFR[0]=low, AFR[1]=high) */
} GPIO_TypeDef;

typedef struct {
    volatile uint32_t CR;            /* 0x00: Clock control */
    volatile uint32_t PLLCFGR;       /* 0x04: PLL config */
    volatile uint32_t CFGR;          /* 0x08: Clock config */
    volatile uint32_t CIR;           /* 0x0C: Clock interrupt */
    volatile uint32_t AHB1RSTR;      /* 0x10: AHB1 reset */
    volatile uint32_t AHB2RSTR;      /* 0x14: AHB2 reset */
    volatile uint32_t AHB3RSTR;      /* 0x18: AHB3 reset */
    uint32_t RESERVED0;              /* 0x1C */
    volatile uint32_t APB1RSTR;      /* 0x20: APB1 reset */
    volatile uint32_t APB2RSTR;      /* 0x24: APB2 reset */
    uint32_t RESERVED1[2];           /* 0x28, 0x2C */
    volatile uint32_t AHB1ENR;       /* 0x30: AHB1 clock enable */
    volatile uint32_t AHB2ENR;       /* 0x34: AHB2 clock enable */
    volatile uint32_t AHB3ENR;       /* 0x38: AHB3 clock enable */
    uint32_t RESERVED2;              /* 0x3C */
    volatile uint32_t APB1ENR;       /* 0x40: APB1 clock enable */
    volatile uint32_t APB2ENR;       /* 0x44: APB2 clock enable */
} RCC_TypeDef;

typedef struct {
    volatile uint32_t SR;   /* 0x00: Status register */
    volatile uint32_t DR;   /* 0x04: Data register */
    volatile uint32_t BRR;  /* 0x08: Baud rate register */
    volatile uint32_t CR1;  /* 0x0C: Control register 1 */
    volatile uint32_t CR2;  /* 0x10: Control register 2 */
    volatile uint32_t CR3;  /* 0x14: Control register 3 */
    volatile uint32_t GTPR; /* 0x18: Guard time and prescaler */
} USART_TypeDef;

typedef struct {
    volatile uint32_t CPUID; /* 0x00 */
    volatile uint32_t ICSR;  /* 0x04 */
    volatile uint32_t VTOR;  /* 0x08: Vector Table Offset Register */
    volatile uint32_t AIRCR; /* 0x0C: Application Interrupt and Reset Control */
    volatile uint32_t SCR;   /* 0x10 */
    volatile uint32_t CCR;   /* 0x14 */
    volatile uint32_t SHPR[3]; /* 0x18-0x20 */
    volatile uint32_t SHCSR; /* 0x24 */
    volatile uint32_t CFSR;  /* 0x28: Configurable Fault Status Register */
    volatile uint32_t HFSR;  /* 0x2C: HardFault Status Register */
    volatile uint32_t DFSR;  /* 0x30 */
    volatile uint32_t MMFAR; /* 0x34: MemManage Fault Address Register */
    volatile uint32_t BFAR;  /* 0x38: BusFault Address Register */
} SCB_TypeDef;

/* ============================================================================
 * Peripheral Declarations
 * ============================================================================ */
#define GPIOA               ((GPIO_TypeDef *) GPIOA_BASE)
#define GPIOB               ((GPIO_TypeDef *) GPIOB_BASE)
#define GPIOC               ((GPIO_TypeDef *) GPIOC_BASE)
#define GPIOD               ((GPIO_TypeDef *) GPIOD_BASE)
#define RCC                 ((RCC_TypeDef  *) RCC_BASE)
#define USART1              ((USART_TypeDef*) USART1_BASE)
#define USART2              ((USART_TypeDef*) USART2_BASE)
#define SCB                 ((SCB_TypeDef  *) SCB_BASE)

/* RCC AHB1ENR Bitfields */
#define RCC_AHB1ENR_GPIOAEN (1UL << 0)
#define RCC_AHB1ENR_GPIOBEN (1UL << 1)
#define RCC_AHB1ENR_GPIOCEN (1UL << 2)
#define RCC_AHB1ENR_GPIODEN (1UL << 3)

/* RCC APB1ENR Bitfields */
#define RCC_APB1ENR_USART2EN (1UL << 17)

/* RCC APB2ENR Bitfields */
#define RCC_APB2ENR_USART1EN (1UL << 4)

/* USART Status Register (SR) Flags */
#define USART_SR_TXE        (1UL << 7) /* Transmit data register empty */
#define USART_SR_TC         (1UL << 6) /* Transmission complete */
#define USART_SR_RXNE       (1UL << 5) /* Read data register not empty */

/* USART Control Register 1 (CR1) Flags */
#define USART_CR1_UE        (1UL << 13) /* USART enable */
#define USART_CR1_TE        (1UL << 3)  /* Transmitter enable */
#define USART_CR1_RE        (1UL << 2)  /* Receiver enable */

#endif /* STM32F446_H */
