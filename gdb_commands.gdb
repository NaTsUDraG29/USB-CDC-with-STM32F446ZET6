# ============================================================================
# GDB Command Script for STM32F446 Bring-Up
# Run with: arm-none-eabi-gdb -x gdb_commands.gdb build/bootloader.elf
# ============================================================================

# 1. Connect to OpenOCD GDB server running on local port 3333
target remote localhost:3333

# 2. Reset the target and halt execution
monitor reset halt

# 3. Flash the binary to STM32 Flash memory
load

# 4. Set useful initial breakpoints
break Reset_Handler
break main
break HardFault_Handler

# 5. Display configuration
set print pretty on
set confirm off
set pagination off

# 6. Helpful aliases for embedded registers
echo \n[GDB] Connected to STM32F446!\n
echo [GDB] Type 'continue' (or 'c') to run to Reset_Handler/main.\n
echo [GDB] Useful commands:\n
echo   - 'info registers' or 'i r'  : show general CPU registers\n
echo   - 'x/16wx 0x08000000'        : examine Vector Table in Flash\n
echo   - 'x/16wx 0x20000000'        : examine RAM\n
echo   - 'step' (s), 'next' (n)     : step line by line\n
echo   - 'stepi' (si)               : step single assembly instruction\n
echo   - 'print/x *USART1'          : inspect USART1 registers\n\n
