@echo off
setlocal

set "COMPILADOR=C:\Program Files\CodeBlocks\MinGW\bin\g++.exe"
set "SFML=dependencias\SFML"

if not exist "%COMPILADOR%" (
    echo No se encontro el compilador de CodeBlocks.
    exit /b 1
)

if not exist "%SFML%\include\SFML\Graphics.hpp" (
    echo No se encontro SFML en dependencias\SFML.
    exit /b 1
)

if not exist "bin\Debug" mkdir "bin\Debug"
if not exist "obj\Debug" mkdir "obj\Debug"

"%COMPILADOR%" -std=c++17 -Wall -Wextra ^
    -Iinclude -I"%SFML%\include" ^
    src\main.cpp src\Juego.cpp ^
    -L"%SFML%\lib" ^
    -lsfml-graphics-d -lsfml-window-d -lsfml-system-d ^
    -o bin\Debug\ScrapProtocol.exe

if errorlevel 1 exit /b 1

copy /Y "%SFML%\bin\*.dll" "bin\Debug\" >nul

echo Compilacion terminada: bin\Debug\ScrapProtocol.exe
exit /b 0

