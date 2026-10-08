@echo off
setlocal

set "COMPILADOR=C:\Program Files\CodeBlocks\MinGW\bin\g++.exe"
set "SFML=dependencias\SFML"

if not exist "%COMPILADOR%" (
    echo No se encontro el compilador de CodeBlocks.
    exit /b 1
)

if not exist "bin\Debug" mkdir "bin\Debug"

"%COMPILADOR%" -std=c++17 -Wall -Wextra ^
    -Iinclude -I"%SFML%\include" ^
    pruebas\Jugador.cpp src\Jugador.cpp ^
    -L"%SFML%\lib" ^
    -lsfml-graphics-d -lsfml-window-d -lsfml-system-d ^
    -o bin\Debug\PruebaJugador.exe

if errorlevel 1 exit /b 1

copy /Y "%SFML%\bin\*.dll" "bin\Debug\" >nul
if errorlevel 1 exit /b 1

bin\Debug\PruebaJugador.exe
exit /b %errorlevel%
