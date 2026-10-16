@echo off
set OPENOCD="C:\ST\STM32CubeIDE_1.19.0\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.openocd.win32_2.4.200.202505051030\tools\bin\openocd.exe"
set SCRIPTS="C:\ST\STM32CubeIDE_1.19.0\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.debug.openocd_2.3.100.202501240831\resources\openocd\st_scripts"

echo ==============================================================
echo Starting OpenOCD GDB Server on localhost:3333...
echo Press Ctrl+C to terminate.
echo ==============================================================
%OPENOCD% -s %SCRIPTS% -f openocd.cfg
pause
