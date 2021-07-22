@echo off

:: define compilation PATH variables
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"

set executable_name=calculator
set flags=/Fe%executable_name% /nologo
set src_files=src\main.c src\calculator.c

cl %flags% %src_files%
