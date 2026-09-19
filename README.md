# Proyecto TPI

Base minima ejecutable del proyecto academico, con nombre temporal.

## Requisitos verificados

- Code::Blocks con MinGW-w64 GCC 14.2.0 UCRT/SEH de 64 bits.
- C++17.
- SFML 3.1.0 para GCC 14.2.0 MinGW, UCRT/SEH, 64 bits.

Es importante que la version y arquitectura del compilador coincidan con el
paquete de SFML.

## Preparar SFML

1. Descargar el paquete oficial `SFML 3.1.0 - GCC 14.2.0 MinGW (SEH)
   (UCRT) - 64-bit`.
2. Extraerlo temporalmente.
3. Copiar la carpeta extraida dentro de `dependencias`.
4. Renombrarla como `SFML`.

La ruta resultante debe contener:

```text
dependencias/SFML/include
dependencias/SFML/lib
dependencias/SFML/bin
```

La dependencia no se guarda en el repositorio. Cada integrante debe instalar
la misma copia localmente.

## Abrir y compilar

1. Abrir `ProyectoTPI.cbp` con Code::Blocks.
2. Elegir el objetivo `Debug`.
3. Compilar el proyecto.
4. Copiar las DLL de `dependencias/SFML/bin` a `bin/Debug`.
5. Ejecutar desde Code::Blocks.

Como alternativa, `compilar_debug.bat` realiza la compilacion y copia las DLL
automaticamente usando el compilador incluido con Code::Blocks.

La prueba es correcta si aparece una ventana gris de 1920 x 1080 con un
cuadrado naranja en el centro. Se puede cerrar con Escape o con el boton de la
ventana.


## Estructura

- `src`: implementaciones `.cpp`.
- `include`: encabezados `.h`.
- `recursos`: sprites, fuentes y sonidos.
- `datos`: archivos binarios generados por el juego.
- `documentos`: decisiones y documentacion del equipo.
- `pruebas`: pruebas aisladas de los modulos.
- `dependencias`: bibliotecas locales no incluidas en el repositorio.
