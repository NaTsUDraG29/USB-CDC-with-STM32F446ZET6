# ============================================================================
# Dual-Binary Makefile: Bootloader (0x08000000) & Application (0x08008000)
# Target: STM32F446ZET6 (BigTreeTech Octopus)
# ============================================================================

BUILD_DIR = build

# Tools
CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE    = arm-none-eabi-size
GDB     = arm-none-eabi-gdb

OPENOCD_BIN = "C:/ST/STM32CubeIDE_1.19.0/STM32CubeIDE/plugins/com.st.stm32cube.ide.mcu.externaltools.openocd.win32_2.4.200.202505051030/tools/bin/openocd.exe"
OPENOCD_SCRIPTS = "C:/ST/STM32CubeIDE_1.19.0/STM32CubeIDE/plugins/com.st.stm32cube.ide.mcu.debug.openocd_2.3.100.202501240831/resources/openocd/st_scripts"
STM32_CLI = "C:/Program Files/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/STM32_Programmer_CLI.exe"

# Architecture Flags
MCU = -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16

# ----------------------------------------------------------------------------
# 1. Bootloader Configuration (32 KB at 0x08000000)
# ----------------------------------------------------------------------------
BOOT_SRCS = bootloader/src/startup.c bootloader/src/main.c
BOOT_INCS = -Ibootloader/inc
BOOT_CFLAGS = $(MCU) -O0 -g3 -Wall -Wextra -ffreestanding -nostdlib $(BOOT_INCS)
BOOT_LDFLAGS = $(MCU) -Tbootloader/bootloader.ld -Wl,-Map=$(BUILD_DIR)/bootloader.map,--cref -nostdlib

# ----------------------------------------------------------------------------
# 2. Application Configuration (480 KB at 0x08008000, USB CDC VCP)
# ----------------------------------------------------------------------------
APP_SRCS = \
	app/src/startup.c \
	app/src/main.c \
	app/src/syscalls.c \
	app/src/system_stm32f4xx.c \
	app/src/usbd_conf.c \
	app/src/usbd_desc.c \
	app/src/usbd_cdc_interface.c \
	app/drivers/HAL/Src/stm32f4xx_hal.c \
	app/drivers/HAL/Src/stm32f4xx_hal_cortex.c \
	app/drivers/HAL/Src/stm32f4xx_hal_rcc.c \
	app/drivers/HAL/Src/stm32f4xx_hal_rcc_ex.c \
	app/drivers/HAL/Src/stm32f4xx_hal_gpio.c \
	app/drivers/HAL/Src/stm32f4xx_hal_flash.c \
	app/drivers/HAL/Src/stm32f4xx_hal_flash_ex.c \
	app/drivers/HAL/Src/stm32f4xx_hal_pwr.c \
	app/drivers/HAL/Src/stm32f4xx_hal_pwr_ex.c \
	app/drivers/HAL/Src/stm32f4xx_hal_pcd.c \
	app/drivers/HAL/Src/stm32f4xx_hal_pcd_ex.c \
	app/drivers/HAL/Src/stm32f4xx_ll_usb.c \
	app/drivers/USB_Device/Core/Src/usbd_core.c \
	app/drivers/USB_Device/Core/Src/usbd_ctlreq.c \
	app/drivers/USB_Device/Core/Src/usbd_ioreq.c \
	app/drivers/USB_Device/Class/CDC/Src/usbd_cdc.c

APP_INCS = \
	-Iapp/inc \
	-Iapp/drivers/CMSIS/Include \
	-Iapp/drivers/CMSIS/Device/Include \
	-Iapp/drivers/HAL/Inc \
	-Iapp/drivers/USB_Device/Core/Inc \
	-Iapp/drivers/USB_Device/Class/CDC/Inc

APP_CFLAGS = $(MCU) -O0 -g3 -DSTM32F446xx -DUSE_USB_FS -DHSE_VALUE=12000000U $(APP_INCS)
APP_LDFLAGS = $(MCU) -Tapp/app.ld -Wl,-Map=$(BUILD_DIR)/app.map,--cref --specs=nano.specs --specs=nosys.specs

# ----------------------------------------------------------------------------
# Rules
# ----------------------------------------------------------------------------
all: $(BUILD_DIR) bootloader app

$(BUILD_DIR):
	cmd /c "if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)"

# Build Bootloader
bootloader: $(BUILD_DIR) $(BUILD_DIR)/bootloader.bin $(BUILD_DIR)/bootloader.hex
	$(SIZE) $(BUILD_DIR)/bootloader.elf

$(BUILD_DIR)/bootloader.elf: $(BOOT_SRCS) bootloader/bootloader.ld
	$(CC) $(BOOT_CFLAGS) $(BOOT_SRCS) $(BOOT_LDFLAGS) -o $@

$(BUILD_DIR)/bootloader.bin: $(BUILD_DIR)/bootloader.elf
	$(OBJCOPY) -O binary $< $@

$(BUILD_DIR)/bootloader.hex: $(BUILD_DIR)/bootloader.elf
	$(OBJCOPY) -O ihex $< $@

# Build Application
app: $(BUILD_DIR) $(BUILD_DIR)/app.bin $(BUILD_DIR)/app.hex
	$(SIZE) $(BUILD_DIR)/app.elf

$(BUILD_DIR)/app.elf: $(APP_SRCS) app/app.ld
	$(CC) $(APP_CFLAGS) $(APP_SRCS) $(APP_LDFLAGS) -o $@

$(BUILD_DIR)/app.bin: $(BUILD_DIR)/app.elf
	$(OBJCOPY) -O binary $< $@

$(BUILD_DIR)/app.hex: $(BUILD_DIR)/app.elf
	$(OBJCOPY) -O ihex $< $@

# ----------------------------------------------------------------------------
# Flashing Targets
# ----------------------------------------------------------------------------
# Flash Bootloader to 0x08000000
flash-bootloader: $(BUILD_DIR)/bootloader.bin
	$(STM32_CLI) -c port=SWD freq=4000 -w $(BUILD_DIR)/bootloader.bin 0x08000000 -v -rst

# Flash Application to 0x08008000
flash-app: $(BUILD_DIR)/app.bin
	$(STM32_CLI) -c port=SWD freq=4000 -w $(BUILD_DIR)/app.bin 0x08008000 -v -rst

# Flash Both (Bootloader then Application, and reset)
flash-all: $(BUILD_DIR)/bootloader.bin $(BUILD_DIR)/app.bin
	$(STM32_CLI) -c port=SWD freq=4000 -w $(BUILD_DIR)/bootloader.bin 0x08000000 -v -w $(BUILD_DIR)/app.bin 0x08008000 -v -rst

# Default flash target
flash: flash-all

# Debug Targets
openocd:
	$(OPENOCD_BIN) -s $(OPENOCD_SCRIPTS) -f openocd.cfg

debug-bootloader: $(BUILD_DIR)/bootloader.elf
	$(GDB) -ex "target remote :3333" -ex "monitor reset halt" $(BUILD_DIR)/bootloader.elf

debug-app: $(BUILD_DIR)/app.elf
	$(GDB) -ex "target remote :3333" -ex "monitor reset halt" $(BUILD_DIR)/app.elf

clean:
	cmd /c "if exist $(BUILD_DIR) rmdir /s /q $(BUILD_DIR)"

.PHONY: all bootloader app flash-bootloader flash-app flash-all flash openocd debug-bootloader debug-app clean
