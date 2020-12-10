@echo off
rem This file is the script to set path for ARM eabi tool chain.

set TL_PATH="C:\Program Files (x86)\GNU Arm Embedded Toolchain\9 2020-q2-update\bin"
set PATH=%TL_PATH%;%PATH%
cmd /K cd %CD% 
