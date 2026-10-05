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
El suelo representa un deposito de chatarra al aire libre. `Escenario.h` y
`Escenario.cpp` administran su textura, guardada en `recursos/entorno`.
La camara sigue al jugador y el suelo se repite en coordenadas de mundo, por
lo que se puede caminar mas alla de los bordes originales, incluso hacia
coordenadas negativas. La interfaz permanece fija en pantalla. R tambien
restablece la camara. La textura completa se reutiliza como mosaico; no es
todavia un mapa de obstaculos ni una generacion de terrenos diferentes.
El arma dispara automaticamente hacia el enemigo vivo visible mas cercano,
dentro de un rango de 600 pixeles. Las balas tambien desaparecen al recorrer
esa distancia maxima. Si un enemigo muere, reaparece en su
posicion inicial despues de 3 segundos. Cuando un enemigo toca al jugador,
aplica dano cada 0.75 segundos y aparece un numero rojo flotante con el valor
recibido. Al morir, deja cuadrados recolectables: celeste para experiencia y
amarillo para chatarra. El jugador debe acercarse a 90 pixeles para recogerlos;
la cantidad obtenida aparece brevemente junto al personaje y los totales se
muestran en la interfaz superior junto con el nombre del arma seleccionada.
Solo para pruebas, 1 selecciona la pistola (25 de dano, 2 disparos por segundo)
y 2 selecciona la metralleta (6 de dano, 8 disparos por segundo). Tambien se
pueden usar las teclas del teclado numerico. La adquisicion definitiva del arma
queda pendiente de integrar con el sistema de progreso. Ambas conservan su
propio enfriamiento al alternar. El arma dispara aunque haya
otros proyectiles en vuelo; cada disparo impacta una sola vez y se elimina al
impactar o salir de la ventana. R
reinicia la prueba, incluida la vida del jugador, y elimina los recursos que
quedaron en el suelo, limpia los disparos y vuelve a seleccionar la pistola;
Escape o el boton de la ventana la cierra. Los proyectiles de pistola son
amarillos de 28 x 14 pixeles y los de metralleta naranjas de 14 x 6; ambos
viajan a 800 pixeles por segundo y conservan su dano y formato al cambiar de arma.

Una estacion de mejoras se encuentra cerca del punto inicial, a la
derecha del jugador. Acercarse y pulsar F gasta chatarra y pausa el combate:
1 elige +20 de vida maxima y actual, 2 elige +20% del dano base de ambas armas
y 3 elige +15% de la velocidad base de ataque de ambas armas. Cada compra
permite una sola eleccion. El costo empieza en 10 y aumenta 60% redondeando
hacia arriba: 10, 16, 26, 42, 68... R devuelve una compra pendiente; las
compras ya confirmadas, sus bonificaciones y el costo se conservan como el
nivel y la chatarra del reinicio actual. La estacion usa un dibujo provisional
de terminal mecanica. `EstacionMejoras.h/.cpp` aislan su logica para adaptarla
al catalogo definitivo del responsable. Los dos metodos nuevos de Jugador
estan autorizados para pruebas y marcados TEMPORAL.

La clase `Arma` conserva las estadisticas del arma y controla el tiempo entre
disparos. La clase `Proyectil` mantiene el movimiento, dano y estado de cada
disparo.

Para verificar los disparos simultaneos, la cadencia, las colisiones, las
recompensas y el reinicio, ejecutar `pruebas/probar_juego_proyectiles.bat`
desde la carpeta del proyecto. Esta prueba abre brevemente una ventana de SFML.

`pruebas/probar_camara.bat` comprueba seguimiento, cobertura del suelo en
coordenadas negativas, anclaje al mundo, proyectiles, reaparicion y reinicio.
`pruebas/probar_estacion_mejoras.bat` comprueba el cobro unico, precio creciente,
pausa, las tres opciones, el reembolso y la conservacion de compras confirmadas.


## Estructura

- `src`: implementaciones `.cpp`.
- `include`: encabezados `.h`.
- `recursos`: sprites, fuentes y sonidos.
- `datos`: archivos binarios generados por el juego.
- `documentos`: decisiones y documentacion del equipo.
- `pruebas`: pruebas aisladas de los modulos.
- `dependencias`: bibliotecas locales no incluidas en el repositorio.

### Estaciones y cofres por zonas (prototipo)
Los objetos se distribuyen al generar cada zona y conservan su estado. F interactua con el mas cercano. Cada estacion permite una compra y queda apagada; cada cofre entrega una recompensa automatica y queda abierto. Los precios de estaciones y cofres aumentan 60% con contadores independientes. Cofres: 15, 24, 39, 63... Ver documentos/encuentros_aleatorios.md. Pruebas: pruebas/probar_encuentros.bat.
