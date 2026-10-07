@echo off
setlocal

set "COMPILADOR=C:\Program Files\CodeBlocks\MinGW\bin\g++.exe"

if not exist "%COMPILADOR%" (
    echo No se encontro el compilador de CodeBlocks.
    exit /b 1
)

if not exist "bin\Debug" mkdir "bin\Debug"

"%COMPILADOR%" -std=c++17 -Wall -Wextra -Iinclude ^
    pruebas\EnemigoProyectil.cpp src\Enemigo.cpp src\Proyectil.cpp src\SistemaColisiones.cpp ^
    -o bin\Debug\PruebaEnemigoProyectil.exe

if errorlevel 1 exit /b 1

bin\Debug\PruebaEnemigoProyectil.exe
exit /b %errorlevel%
