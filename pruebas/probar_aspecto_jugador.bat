@echo off
setlocal
if not defined TPI_CARPETA_DATOS set "TPI_CARPETA_DATOS=%CD%\bin\Debug\datos_pruebas\%~n0_%RANDOM%_%RANDOM%"
if not exist "%TPI_CARPETA_DATOS%" mkdir "%TPI_CARPETA_DATOS%"
set "BIN_COMPILADOR=C:\Program Files\CodeBlocks\MinGW\bin"
set "SFML=dependencias\SFML"
if not exist "bin\Debug" mkdir "bin\Debug"
"%BIN_COMPILADOR%\g++.exe" -std=c++17 -Wall -Wextra ^
    -Iinclude -I"%SFML%\include" ^
    pruebas\AspectoJugador.cpp src\AspectoJugador.cpp src\AspectoArmas.cpp src\Jugador.cpp src\ArchivoJugadores.cpp src\ArchivoMejoras.cpp ^
    -L"%SFML%\lib" -lsfml-graphics-d -lsfml-window-d -lsfml-system-d ^
    -o bin\Debug\PruebaAspectoJugador.exe
if errorlevel 1 exit /b 1
copy /Y "%SFML%\bin\*.dll" "bin\Debug\" >nul
if errorlevel 1 exit /b 1
for %%D in (libgcc_s_seh-1.dll libstdc++-6.dll libwinpthread-1.dll) do (
    copy /Y "%BIN_COMPILADOR%\%%D" "bin\Debug\" >nul
    if errorlevel 1 exit /b 1
)
bin\Debug\PruebaAspectoJugador.exe
exit /b %errorlevel%
