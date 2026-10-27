@echo off
setlocal
if not defined TPI_CARPETA_DATOS set "TPI_CARPETA_DATOS=%CD%\bin\Debug\datos_pruebas\%~n0_%RANDOM%_%RANDOM%"
if not exist "%TPI_CARPETA_DATOS%" mkdir "%TPI_CARPETA_DATOS%"

set "COMPILADOR=C:\Program Files\CodeBlocks\MinGW\bin\g++.exe"
set "SFML=dependencias\SFML"

if not exist "%COMPILADOR%" (
    echo No se encontro el compilador de CodeBlocks.
    exit /b 1
)

if not exist "bin\Debug" mkdir "bin\Debug"

"%COMPILADOR%" -std=c++17 -Wall -Wextra ^
    -Iinclude -I"%SFML%\include" ^
    pruebas\Jugador.cpp src\Jugador.cpp src\ArchivoJugadores.cpp src\ArchivoMejoras.cpp ^
    -L"%SFML%\lib" ^
    -lsfml-graphics-d -lsfml-window-d -lsfml-system-d ^
    -o bin\Debug\PruebaJugador.exe

if errorlevel 1 exit /b 1

copy /Y "%SFML%\bin\*.dll" "bin\Debug\" >nul
if errorlevel 1 exit /b 1

bin\Debug\PruebaJugador.exe
exit /b %errorlevel%
