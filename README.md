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

La prueba visual temporal muestra al jugador y tres enemigos que lo persiguen.
El arma dispara automaticamente hacia el enemigo vivo mas cercano y su alcance
cubre la diagonal completa de la ventana. Si un enemigo muere, reaparece en su
posicion inicial despues de 3 segundos. Cuando un enemigo toca al jugador,
aplica dano cada 0.75 segundos y aparece un numero rojo flotante con el valor
recibido. Al morir, deja cuadrados recolectables: celeste para experiencia y
amarillo para chatarra. El jugador debe acercarse a 90 pixeles para recogerlos;
la cantidad obtenida aparece brevemente junto al personaje y los totales se
muestran en la interfaz superior. Solo existe un proyectil activo a la vez. R
reinicia la prueba, incluida la vida del jugador, y elimina los recursos que
quedaron en el suelo; Escape o el boton de la ventana la cierra.

La clase `Arma` conserva las estadisticas del arma y controla el tiempo entre
disparos. La clase `Proyectil` mantiene el movimiento, dano y estado de cada
disparo.


## Estructura

- `src`: implementaciones `.cpp`.
- `include`: encabezados `.h`.
- `recursos`: sprites, fuentes y sonidos.
- `datos`: archivos binarios generados por el juego.
- `documentos`: decisiones y documentacion del equipo.
- `pruebas`: pruebas aisladas de los modulos.
- `dependencias`: bibliotecas locales no incluidas en el repositorio.
