cmake_minimum_required(VERSION 3.20)

if(NOT CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/arm-toolchain.cmake")
endif()

project(stm32f446_dual_binary C ASM)

set(STM32_CLI "C:/Program Files/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/STM32_Programmer_CLI.exe")
set(OPENOCD_BIN "C:/ST/STM32CubeIDE_1.19.0/STM32CubeIDE/plugins/com.st.stm32cube.ide.mcu.externaltools.openocd.win32_2.4.200.202505051030/tools/bin/openocd.exe")
set(OPENOCD_SCRIPTS "C:/ST/STM32CubeIDE_1.19.0/STM32CubeIDE/plugins/com.st.stm32cube.ide.mcu.debug.openocd_2.3.100.202501240831/resources/openocd/st_scripts")

set(MCU_FLAGS
    -mcpu=cortex-m4
    -mthumb
    -mfloat-abi=hard
    -mfpu=fpv4-sp-d16
)

# ============================================================================
# 1. Target: Bootloader (0x08000000, 32KB)
# ============================================================================
add_executable(bootloader.elf
    bootloader/src/startup.c
    bootloader/src/main.c
)
target_include_directories(bootloader.elf PRIVATE bootloader/inc)
target_compile_options(bootloader.elf PRIVATE
    ${MCU_FLAGS}
    -O0 -g3 -Wall -Wextra -ffreestanding -nostdlib
)
target_link_options(bootloader.elf PRIVATE
    ${MCU_FLAGS}
    -T${CMAKE_CURRENT_SOURCE_DIR}/bootloader/bootloader.ld
    -Wl,-Map=bootloader.map,--cref
    -nostdlib
)

add_custom_command(TARGET bootloader.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary bootloader.elf bootloader.bin
    COMMAND ${CMAKE_OBJCOPY} -O ihex bootloader.elf bootloader.hex
    COMMAND ${CMAKE_SIZE} bootloader.elf
    COMMENT "Built bootloader.bin"
)

# ============================================================================
# 2. Target: Application (0x08008000, 480KB, USB CDC VCP)
# ============================================================================
set(APP_SRCS
    app/src/startup.c
    app/src/main.c
    app/src/syscalls.c
    app/src/system_stm32f4xx.c
    app/src/usbd_conf.c
    app/src/usbd_desc.c
    app/src/usbd_cdc_interface.c
    app/drivers/HAL/Src/stm32f4xx_hal.c
    app/drivers/HAL/Src/stm32f4xx_hal_cortex.c
    app/drivers/HAL/Src/stm32f4xx_hal_rcc.c
    app/drivers/HAL/Src/stm32f4xx_hal_rcc_ex.c
    app/drivers/HAL/Src/stm32f4xx_hal_gpio.c
    app/drivers/HAL/Src/stm32f4xx_hal_flash.c
    app/drivers/HAL/Src/stm32f4xx_hal_flash_ex.c
    app/drivers/HAL/Src/stm32f4xx_hal_pwr.c
    app/drivers/HAL/Src/stm32f4xx_hal_pwr_ex.c
    app/drivers/HAL/Src/stm32f4xx_hal_pcd.c
    app/drivers/HAL/Src/stm32f4xx_hal_pcd_ex.c
    app/drivers/HAL/Src/stm32f4xx_ll_usb.c
    app/drivers/USB_Device/Core/Src/usbd_core.c
    app/drivers/USB_Device/Core/Src/usbd_ctlreq.c
    app/drivers/USB_Device/Core/Src/usbd_ioreq.c
    app/drivers/USB_Device/Class/CDC/Src/usbd_cdc.c
)

add_executable(app.elf ${APP_SRCS})
target_compile_definitions(app.elf PRIVATE STM32F446xx USE_USB_FS HSE_VALUE=12000000U)
target_include_directories(app.elf PRIVATE
    app/inc
    app/drivers/CMSIS/Include
    app/drivers/CMSIS/Device/Include
    app/drivers/HAL/Inc
    app/drivers/USB_Device/Core/Inc
    app/drivers/USB_Device/Class/CDC/Inc
)
target_compile_options(app.elf PRIVATE
    ${MCU_FLAGS}
    -O0 -g3 -Wall -Wextra
)
target_link_options(app.elf PRIVATE
    ${MCU_FLAGS}
    -T${CMAKE_CURRENT_SOURCE_DIR}/app/app.ld
    -Wl,-Map=app.map,--cref
    --specs=nano.specs
    --specs=nosys.specs
)

add_custom_command(TARGET app.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary app.elf app.bin
    COMMAND ${CMAKE_OBJCOPY} -O ihex app.elf app.hex
    COMMAND ${CMAKE_SIZE} app.elf
    COMMENT "Built app.bin"
)

# ============================================================================
# Flashing Targets
# ============================================================================
add_custom_target(flash-bootloader
    COMMAND ${STM32_CLI} -c port=SWD freq=4000 -w ${CMAKE_CURRENT_BINARY_DIR}/bootloader.bin 0x08000000 -v -rst
    DEPENDS bootloader.elf
    COMMENT "Flashing bootloader.bin to 0x08000000"
)

add_custom_target(flash-app
    COMMAND ${STM32_CLI} -c port=SWD freq=4000 -w ${CMAKE_CURRENT_BINARY_DIR}/app.bin 0x08008000 -v -rst
    DEPENDS app.elf
    COMMENT "Flashing app.bin to 0x08008000"
)

add_custom_target(flash
    COMMAND ${STM32_CLI} -c port=SWD freq=4000 -w ${CMAKE_CURRENT_BINARY_DIR}/bootloader.bin 0x08000000 -v -w ${CMAKE_CURRENT_BINARY_DIR}/app.bin 0x08008000 -v -rst
    DEPENDS bootloader.elf app.elf
    COMMENT "Flashing both bootloader and app"
)
