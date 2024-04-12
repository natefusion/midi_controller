@ECHO OFF

rem https://www.microchip.com/en-us/tools-resources/develop/microchip-studio/gcc-compilers
set "toolchain_path=devkit\avr-toolchain\bin"

rem https://github.com/avrdudes/avrdude/releases/tag/v7.3
set "avrdude_path=devkit\avrdude"

rem https://gnuwin32.sourceforge.net/packages/make.htm
set "devkit_path=devkit\make\bin"

set PATH=%toolchain_path%;%avrdude_path%;%devkit_path%;%PATH%

start cmd /k
