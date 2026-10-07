@echo off
setlocal
cd /d "%~dp0.."

set "COMPILADOR=C:\Program Files\CodeBlocks\MinGW\bin"
if not exist "%COMPILADOR%\g++.exe" (
    echo No se encontro el compilador de CodeBlocks.
    exit /b 1
)

if not exist "bin\Debug" mkdir "bin\Debug"
"%COMPILADOR%\g++.exe" -std=c++17 -Wall -Wextra -Werror -Iinclude ^
    pruebas\SistemaColisiones.cpp src\SistemaColisiones.cpp ^
    -o bin\Debug\PruebaSistemaColisiones.exe
if errorlevel 1 exit /b 1

set "PATH=%COMPILADOR%;%PATH%"
bin\Debug\PruebaSistemaColisiones.exe
exit /b %ERRORLEVEL%
