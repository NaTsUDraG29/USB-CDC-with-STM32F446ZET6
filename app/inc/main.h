#ifndef __MAIN_H
#define __MAIN_H

#include "stm32f4xx_hal.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_cdc.h"
#include "usbd_cdc_interface.h"

void Error_Handler(void);
void uart_puts(const char *s);

#endif /* __MAIN_H */
