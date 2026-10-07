@echo off
setlocal
set "COMPILADOR=C:\Program Files\CodeBlocks\MinGW\bin\g++.exe"
if not exist "%COMPILADOR%" exit /b 1
if not exist "bin\Debug" mkdir "bin\Debug"
"%COMPILADOR%" -std=c++17 -Wall -Wextra -Iinclude pruebas\InventarioArmas.cpp src\InventarioArmas.cpp src\EstacionMejoras.cpp src\Arma.cpp src\Proyectil.cpp -o bin\Debug\PruebaInventarioArmas.exe
if errorlevel 1 exit /b 1
bin\Debug\PruebaInventarioArmas.exe
exit /b %errorlevel%
