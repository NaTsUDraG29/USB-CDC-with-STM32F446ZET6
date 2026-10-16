@echo off
set STM32_CLI="C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe"

echo ==============================================================
echo Flashing STM32F446ZET6 via ST-Link (STM32CubeProgrammer CLI)
echo ==============================================================
%STM32_CLI% -c port=SWD freq=4000 -w build\bootloader.bin 0x08000000 -v -rst
pause
