@echo off
setlocal

set "BIN_COMPILADOR=C:\Program Files\CodeBlocks\MinGW\bin"
set "COMPILADOR=%BIN_COMPILADOR%\g++.exe"
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
    src\main.cpp src\Juego.cpp src\AspectoEncuentros.cpp src\AspectoProyectiles.cpp src\AspectoArmas.cpp src\Escenario.cpp src\EstacionMejoras.cpp src\InventarioArmas.cpp src\EncuentroAleatorio.cpp src\Jugador.cpp src\Enemigo.cpp src\GestorOleadas.cpp src\Partida.cpp src\Arma.cpp src\Proyectil.cpp src\SistemaColisiones.cpp ^
    -L"%SFML%\lib" ^
    -lsfml-graphics-d -lsfml-window-d -lsfml-system-d ^
    -o bin\Debug\ProyectoTPI.exe

if errorlevel 1 exit /b 1

copy /Y "%SFML%\bin\*.dll" "bin\Debug\" >nul
if errorlevel 1 exit /b 1

for %%D in (libgcc_s_seh-1.dll libstdc++-6.dll libwinpthread-1.dll) do (
    copy /Y "%BIN_COMPILADOR%\%%D" "bin\Debug\" >nul
    if errorlevel 1 exit /b 1
)

echo Compilacion terminada: bin\Debug\ProyectoTPI.exe
exit /b 0
