# STM32F446ZET6 Bootloader and USB Virtual COM Port

Firmware bring-up project for the **STM32F446ZET6**. It builds a small flash-resident bootloader and an application that provides a USB full-speed CDC virtual COM port, UART diagnostic output, and a one-second heartbeat.

## Features

- Stage-1 bootloader checks the application vector table and jumps to a valid application after a short delay.
- Application runs at 168 MHz and initializes USB full-speed using a 48 MHz clock.
- USB CDC sends a heartbeat/status line once per second and echoes data received from the host.
- Diagnostic messages are mirrored to USART1 and USART2 transmit pins.
- Build outputs include ELF, BIN, and Intel HEX files.

## Hardware

- STM32F446ZET6 microcontroller and a board that provides its required power, clock, and USB connections.
- An external **12 MHz HSE crystal/clock source** is expected by the firmware clock configuration. The application has an HSI fallback if HSE startup fails.
- ST-Link (V2, V2-1, or V3) and SWD connection for programming/debugging.
- USB data cable connected to the MCU's USB OTG FS D-/D+ signals for the CDC interface.
- Optional 3.3 V TTL UART adapters or host devices for serial diagnostics. Share ground with the target; do not connect RS-232 voltage levels directly.

### Pin configuration

| Function | STM32 pin(s) | Configuration / notes |
| --- | --- | --- |
| USB OTG FS data | PA11 (DM), PA12 (DP) | USB CDC device interface. The application briefly drives PA12 low during USB re-enumeration. |
| USART1 diagnostics | PA9 (TX) | AF7, 115200 baud, 8-N-1 in the application. |
| USART1 receive | PA10 (RX) | AF7 and configured in the bootloader; the current application does not use UART receive. |
| USART2 diagnostics | PD5 (TX) | AF7, 115200 baud, 8-N-1 in the application. |
| USART2 receive | PD6 (RX) | AF7 and configured in the bootloader; the current application does not use UART receive. |
| SWD programming/debug | PA13 (SWDIO), PA14 (SWCLK) | Connect to a compatible ST-Link, along with target ground and the probe's reference voltage as required. |

The firmware sends the same UART diagnostic text on PA9 and PD5. UART receive pins are not used by the current bootloader/application behavior.

## Flash layout

| Image | Flash address | Reserved size |
| --- | --- | --- |
| Bootloader | `0x08000000` | 32 KiB |
| Application | `0x08008000` | 480 KiB |

The bootloader expects the application vector table at `0x08008000`. Flash both images for a normal boot. To update only the application, preserve the bootloader and flash the application at its specified address.

## Prerequisites

- **GNU Arm Embedded Toolchain** with `arm-none-eabi-gcc`, `arm-none-eabi-objcopy`, `arm-none-eabi-size`, and (for debugging) `arm-none-eabi-gdb`.
- **CMake 3.20 or newer** and GNU Make for the CMake workflow.
- Alternatively, GNU Make for the Makefile workflow.
- **STM32CubeProgrammer CLI** for the `flash` targets.
- **OpenOCD** and an ST-Link for the OpenOCD/GDB debugging workflow.
- A serial terminal application such as PuTTY or Tera Term to view USB CDC/UART output.

The provided Windows CMake preset and helper scripts contain paths to local STM32CubeIDE and STM32CubeProgrammer installations. If they differ on your computer, update the relevant paths in `CMakePresets.json`, `CMakeLists.txt`, `Makefile`, or the helper `.bat` files before using those workflows.

## Build

### CMake

From the repository root in a terminal with CMake and the ARM toolchain available:

```powershell
cmake --preset default
cmake --build --preset default
```

The preset writes generated files under `build_cmake`.

### GNU Make

From the repository root in a terminal where GNU Make and the ARM toolchain are available:

```powershell
make
```

This builds the bootloader and application into `build`. On Windows, `make.cmd` and `build.bat` are provided as wrappers for the configured local Make executable.

## Flash

Connect the ST-Link to the target using SWD, then use one of the following:

```powershell
# Makefile workflow: build and flash both images
make flash

# CMake workflow: build and flash both images
cmake --build build_cmake --target flash
```

Individual Makefile targets are available when needed:

```powershell
make flash-bootloader
make flash-app
```

The flashing commands use STM32CubeProgrammer CLI and write the images to `0x08000000` and `0x08008000`, respectively.

## USB CDC and serial output

After a successful boot, connect the USB data cable and open the newly enumerated virtual COM port in a terminal. The firmware reports USB state and sends a heartbeat approximately once per second. The CDC interface is configured for **115200 baud, 8 data bits, no parity, 1 stop bit (8-N-1)**; USB CDC baud settings are host-side line coding and do not set the USB bus speed.

For UART diagnostics, connect a 3.3 V TTL adapter to either PA9 (USART1 TX) or PD5 (USART2 TX), connect grounds, and configure the terminal for **115200 8-N-1**. The bootloader also configures PA10 and PD6 as UART receive pins, but the current code does not use incoming UART data.

## Debug with OpenOCD and GDB

1. Start the OpenOCD server with `start_openocd.bat` (or the `make openocd` target).
2. In another terminal, run `start_gdb.bat` to connect GDB to the server and load the bootloader ELF.

The OpenOCD configuration expects an ST-Link and SWD. Adjust the OpenOCD executable/scripts paths in `Makefile` or `start_openocd.bat` if needed.

## Repository layout

- `bootloader/` — stage-1 bootloader source and linker script.
- `app/` — application, STM32 HAL/CMSIS, and USB device stack.
- `CMakeLists.txt`, `CMakePresets.json` — CMake build and flash targets.
- `Makefile` — GNU Make build, flash, and debug targets.
- `openocd.cfg`, `gdb_commands.gdb` — OpenOCD and GDB configuration.
