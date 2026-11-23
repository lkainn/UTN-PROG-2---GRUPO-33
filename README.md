# El Último Recolector

## Requisitos

- Windows de 64 bits.
- Code::Blocks con MinGW-w64 GCC 14.2.0 (UCRT/SEH, 64 bits).
- SFML 3.1.0 para GCC 14.2.0 MinGW (UCRT, 64 bits). [Descarga oficial](https://www.sfml-dev.org/download/sfml/3.1.0/)

## Compilar y ejecutar

1. Extraer SFML en `dependencias/SFML`. Dentro deben quedar las carpetas `include`, `lib` y `bin`.
2. Ejecutar `compilar_debug.bat` desde la carpeta del proyecto. Si Code::Blocks está instalado en otra ubicación, ajustar `BIN_COMPILADOR` en el script para que apunte a la carpeta `bin` de MinGW.
3. Abrir `bin/Debug/ProyectoTPI.exe` y elegir **Nueva partida** en el menú.
