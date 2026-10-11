@echo off
setlocal
set "BIN_COMPILADOR=C:\Program Files\CodeBlocks\MinGW\bin"
set "COMPILADOR=%BIN_COMPILADOR%\g++.exe"
set "SFML=dependencias\SFML"
if not exist "%COMPILADOR%" exit /b 1
if not exist "bin\Debug" mkdir "bin\Debug"
"%COMPILADOR%" -std=c++17 -Wall -Wextra ^
    -Iinclude -I"%SFML%\include" ^
    pruebas\AspectoEncuentros.cpp src\AspectoEncuentros.cpp ^
    -L"%SFML%\lib" -lsfml-graphics-d -lsfml-window-d -lsfml-system-d ^
    -o bin\Debug\PruebaAspectoEncuentros.exe
if errorlevel 1 exit /b 1
copy /Y "%SFML%\bin\*.dll" "bin\Debug\" >nul
if errorlevel 1 exit /b 1
for %%D in (libgcc_s_seh-1.dll libstdc++-6.dll libwinpthread-1.dll) do (
    copy /Y "%BIN_COMPILADOR%\%%D" "bin\Debug\" >nul
    if errorlevel 1 exit /b 1
)
bin\Debug\PruebaAspectoEncuentros.exe
exit /b %errorlevel%
