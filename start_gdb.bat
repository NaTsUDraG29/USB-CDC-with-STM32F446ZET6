@echo off
echo ==============================================================
echo Launching GDB Client connected to OpenOCD...
echo Ensure start_openocd.bat is running in another terminal!
echo ==============================================================
arm-none-eabi-gdb -x gdb_commands.gdb build\bootloader.elf
pause
