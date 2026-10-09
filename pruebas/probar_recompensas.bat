@echo off
setlocal
cd /d "%~dp0.."

set "COMPILADOR=C:\Program Files\CodeBlocks\MinGW\bin\g++.exe"

if not exist "%COMPILADOR%" (
    echo No se encontro el compilador de CodeBlocks.
    exit /b 1
)

if not exist "bin\Debug" mkdir "bin\Debug"

"%COMPILADOR%" -std=c++17 -Wall -Wextra -Werror -Iinclude ^
    pruebas\Recompensas.cpp src\Enemigo.cpp ^
    -o bin\Debug\PruebaRecompensas.exe

if errorlevel 1 exit /b 1

bin\Debug\PruebaRecompensas.exe
exit /b %errorlevel%
