/* ============================================================================
 * STM32F446ZET6 (BigTreeTech Octopus) Application with USB-C Virtual COM Port
 * ============================================================================
 * Partition Base : 0x08008000
 * USB Interface  : PA11 (OTG_FS_DM), PA12 (OTG_FS_DP) -> USB-C Port
 * Clock Config   : 12 MHz HSE Crystal -> 168 MHz SYSCLK, 48 MHz USB Clock
 * ============================================================================ */

#include "main.h"
#include <stdio.h>
#include <string.h>

USBD_HandleTypeDef USBD_Device;

/* Optional Hardware UART for dual logging (PA9 = TFT, PD5 = RPi) */
static void uart_debug_init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_USART2_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* PA9 (TX1) */
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* PD5 (TX2) */
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* USART1 & USART2 @ 115200 (PCLK2 = 84MHz, PCLK1 = 42MHz) */
    USART1->BRR = 0x02D9; /* 84MHz / (16 * 115200) = 45.57 -> 0x2D9 */
    USART1->CR1 = USART_CR1_UE | USART_CR1_TE;

    USART2->BRR = 0x016C; /* 42MHz / (16 * 115200) = 22.78 -> 0x16C */
    USART2->CR1 = USART_CR1_UE | USART_CR1_TE;
}

void uart_puts(const char *s)
{
    while (*s) {
        if (*s == '\n') {
            while (!(USART1->SR & USART_SR_TXE));
            USART1->DR = '\r';
            while (!(USART2->SR & USART_SR_TXE));
            USART2->DR = '\r';
        }
        while (!(USART1->SR & USART_SR_TXE));
        USART1->DR = (uint8_t)*s;
        while (!(USART2->SR & USART_SR_TXE));
        USART2->DR = (uint8_t)*s;
        s++;
    }
}

/* ============================================================================
 * System Clock Configuration:
 * - External Crystal (HSE): 12 MHz on BTT Octopus
 * - PLL_M = 12 (1 MHz input to PLL)
 * - PLL_N = 336 (336 MHz VCO)
 * - PLL_P = 2 -> 168 MHz SYSCLK (HCLK)
 * - PLL_Q = 7 -> 48 MHz USB OTG FS Clock!
 * ============================================================================ */
/* ============================================================================
 * System Clock Configuration:
 * - External Crystal (HSE): 12 MHz on BTT Octopus
 * - PLL_M = 12 (1 MHz input to PLL)
 * - PLL_N = 336 (336 MHz VCO)
 * - PLL_P = 2 -> 168 MHz SYSCLK (HCLK)
 * - PLL_Q = 7 -> 48 MHz USB OTG FS Clock!
 * - STM32F446 Dedicated 48 MHz Clock Mux (DCKCFGR2): selects PLLQ
 * ============================================================================ */
static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /* Configure HSE (12 MHz on Octopus) with PLL */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 12;
    RCC_OscInitStruct.PLL.PLLN = 336;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 7;
    RCC_OscInitStruct.PLL.PLLR = 2;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        /* Fallback to HSI (16 MHz) if HSE fails */
        RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
        RCC_OscInitStruct.HSIState = RCC_HSI_ON;
        RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
        RCC_OscInitStruct.PLL.PLLM = 16;
        RCC_OscInitStruct.PLL.PLLN = 336;
        RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
        RCC_OscInitStruct.PLL.PLLQ = 7;
        HAL_RCC_OscConfig(&RCC_OscInitStruct);
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4; /* 42 MHz */
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2; /* 84 MHz */

    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);

    /* Configure 48 MHz peripheral clock source for STM32F446 (DCKCFGR2: CK48MSEL = PLLQ) */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_CLK48;
    PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48CLKSOURCE_PLLQ;
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
}

void Error_Handler(void)
{
    while (1);
}

int main(void)
{
    /* 1. Point Vector Table to Application Address in Flash (0x08008000) */
    SCB->VTOR = 0x08008000UL;

    /* 2. Re-enable global interrupts early:
     * When jumping from the bootloader, interrupts were disabled with 'cpsid i'.
     * Re-enabling them allows SysTick (uwTick / HAL_Delay) and NVIC handlers to work. */
    __enable_irq();

    /* 3. Initialize HAL Driver (starts SysTick at 1ms) */
    HAL_Init();

    /* 4. Configure System Clocks (168 MHz CPU, 48 MHz USB OTG FS via PLLQ) */
    SystemClock_Config();

    /* 5. Initialize UART for diagnostic logging (PA9=TFT, PD5=RPi) */
    uart_debug_init();
    uart_puts("\r\n");
    uart_puts("==============================================================\r\n");
    uart_puts("   BIGTREETECH OCTOPUS STM32F446 APPLICATION (STAGE 2)        \r\n");
    uart_puts("==============================================================\r\n");
    uart_puts("[APP] Application running at Flash 0x08008000\r\n");
    uart_puts("[APP] Clocks: SYSCLK=168MHz | USB=48MHz (PLLQ)\r\n");

    /* 6. Force USB Host Re-Enumeration:
     * Drive PA12 (USB D+) LOW for 100ms to signal a physical disconnect (SE0)
     * to the PC host USB hub. This clears any stale state from previous runs. */
    uart_puts("[APP] Forcing USB disconnect on PA12 (D+ LOW for 100ms)...\r\n");
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef gpio_disc = {0};
    gpio_disc.Pin = GPIO_PIN_12;
    gpio_disc.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_disc.Pull = GPIO_NOPULL;
    gpio_disc.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &gpio_disc);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
    HAL_Delay(100);

    /* Release PA12 to high-impedance input before USB peripheral takes control */
    gpio_disc.Mode = GPIO_MODE_INPUT;
    HAL_GPIO_Init(GPIOA, &gpio_disc);
    HAL_Delay(10);

    /* 7. Initialize USB Device Stack as CDC Virtual COM Port */
    uart_puts("[APP] Initializing USB CDC stack (PA11=DM, PA12=DP)...\r\n");
    USBD_Init(&USBD_Device, &VCP_Desc, 0);
    USBD_RegisterClass(&USBD_Device, &USBD_CDC);
    USBD_CDC_RegisterInterface(&USBD_Device, &USBD_CDC_fops);
    USBD_Start(&USBD_Device);

    uart_puts("[APP] USB CDC Started! Listening for host enumeration...\r\n");
    uart_puts("[APP] PuTTY: Connect to the assigned COM Port at 115200 8N1\r\n");
    uart_puts("==============================================================\r\n\r\n");

    uint32_t heartbeat = 0;
    char msg[160];

    while (1) {
        HAL_Delay(1000);
        heartbeat++;

        /* Map USBD device state to human-readable string */
        const char *usb_state_str = "DISCONNECTED";
        if (USBD_Device.dev_state == USBD_STATE_CONFIGURED) {
            usb_state_str = "CONFIGURED (READY)";
        } else if (USBD_Device.dev_state == USBD_STATE_ADDRESSED) {
            usb_state_str = "ADDRESSED";
        } else if (USBD_Device.dev_state == USBD_STATE_DEFAULT) {
            usb_state_str = "DEFAULT (RESET)";
        } else if (USBD_Device.dev_state == USBD_STATE_SUSPENDED) {
            usb_state_str = "SUSPENDED";
        }

        /* Format periodic heartbeat log */
        snprintf(msg, sizeof(msg),
                 "[APP #%lu] Hello from BTT Octopus STM32F446! [USB: %s]\r\n",
                 heartbeat, usb_state_str);

        /* Send over USB-C CDC Virtual COM Port if configured by host */
        if (USBD_Device.dev_state == USBD_STATE_CONFIGURED) {
            CDC_Transmit_FS((uint8_t*)msg, (uint16_t)strlen(msg));
        }

        /* Mirror to hardware UART for continuous monitoring */
        uart_puts(msg);
    }

    return 0;
}
