# Proyecto TPI

Prototipo ejecutable del Proyecto TPI, con nombre temporal.

## Requisitos verificados

- Code::Blocks con MinGW-w64 GCC 14.2.0 UCRT/SEH de 64 bits.
- C++17.
- SFML 3.1.0 para GCC 14.2.0 MinGW, UCRT/SEH, 64 bits.

La versión y la arquitectura del compilador deben coincidir con el paquete
local de SFML.

## Preparar SFML

1. Descargar el paquete oficial `SFML 3.1.0 - GCC 14.2.0 MinGW (SEH) (UCRT) - 64-bit`.
2. Extraerlo temporalmente.
3. Copiar la carpeta extraída dentro de `dependencias`.
4. Renombrarla como `SFML`.

La ruta resultante debe contener:

```text
dependencias/SFML/include
dependencias/SFML/lib
dependencias/SFML/bin
```

La dependencia no se guarda en el repositorio. Cada integrante debe instalar
la misma versión localmente.

## Abrir y compilar

1. Abrir `ProyectoTPI.cbp` con Code::Blocks.
2. Elegir el objetivo `Debug`.
3. Compilar el proyecto.
4. Copiar las DLL de `dependencias/SFML/bin` a `bin/Debug`.
5. Ejecutar desde Code::Blocks.

Como alternativa, ejecutar `compilar_debug.bat` desde la raíz del proyecto.
Compila y copia las DLL de SFML y MinGW automáticamente usando el compilador
de `C:\Program Files\CodeBlocks\MinGW\bin`. Después se puede abrir
`bin/Debug/ProyectoTPI.exe` directamente.

## Prototipo actual

`GestorOleadas` administra la aparición de enemigos alrededor del jugador,
con dificultad creciente durante los primeros cinco minutos. La interfaz
muestra el número de oleada y los segundos de partida. Los rápidos se habilitan
desde el segundo 60 y los resistentes desde el 180. R limpia los enemigos y
reinicia el reloj de oleadas; el menú de mejoras pausa el combate.

El suelo representa un depósito de chatarra al aire libre. `Escenario.h` y
`Escenario.cpp` administran su textura, guardada en `recursos/entorno`.
La cámara sigue al jugador y el suelo se repite en coordenadas de mundo,
incluidas las negativas. La interfaz permanece fija en pantalla. R también
restablece la cámara. El suelo todavía no incluye obstáculos con colisión.

El arma dispara automáticamente hacia el enemigo vivo visible más cercano
dentro de su alcance: pistola y metralleta, 600 px; escopeta, 300 px; cañón de
energía, 800 px. Los proyectiles desaparecen al agotar el alcance o salir de
la vista. Cada arma conserva su enfriamiento al alternar y puede disparar
mientras otros proyectiles siguen en vuelo.

El contacto con enemigos aplica daño con 0,75 segundos de invulnerabilidad
entre impactos y muestra un número rojo flotante. Al morir, los enemigos dejan
recursos: celeste para experiencia y amarillo para chatarra. Se recogen al
acercarse a 90 px; la cantidad obtenida aparece junto al personaje y los
acumulados se muestran en la interfaz.

### Controles

| Tecla | Acción |
|---|---|
| WASD | Mover al jugador |
| F | Interactuar con la estación o cofre disponible más cercano, dentro de 100 px |
| 1 / 2 | Seleccionar un slot de arma fuera del menú |
| 1 / 2 / 3 | Elegir una oferta de la estación |
| 1 / 2 en la confirmación | Confirmar el reemplazo de pistola / volver a las ofertas |
| Escape | Cancelar una compra pendiente y devolver el pago; fuera del menú, cerrar la ventana |
| R | Reiniciar la prueba conservando inventario, mejoras, precios y encuentros usados |
| K | Recibir 20 de daño para pruebas, fuera del menú |
| E | Obtener 40 de experiencia para pruebas, fuera del menú |

También se admiten las teclas numéricas del teclado numérico. R restaura la
vida, limpia disparos y recursos del suelo, reinicia las recargas y selecciona
el slot 1 actual. No recupera una pistola descartada.

### Armas y gráficos

Se inicia con pistola en el primer slot y el segundo vacío. Las demás armas
se adquieren en estaciones junto con los beneficios de la oferta elegida.
Solo se dispara el arma seleccionada; el uso simultáneo de dos armas sigue
pendiente.

| Arma | Daño base por proyectil | Disparos/s | Alcance | Descarga base |
|---|---:|---:|---:|---|
| Pistola | 25 | 2 | 600 px | Un proyectil, un impacto |
| Metralleta | 6 | 8 | 600 px | Un proyectil, un impacto |
| Escopeta | 12 | 0,8 | 300 px | Cinco perdigones, un impacto cada uno |
| Cañón de energía | 60 | 0,5 | 800 px | Un proyectil, hasta tres enemigos distintos |

Las mejoras especiales permiten hasta cinco proyectiles de pistola o metralleta,
nueve perdigones de escopeta y seis impactos de energía. El cañón no repite daño
sobre el mismo enemigo. Los disparos en vuelo conservan sus propiedades al
mejorar o cambiar de arma.

Las cuatro armas y sus proyectiles utilizan PNG transparentes. El arma equipada
se dibuja junto al jugador y gira hacia el objetivo automático; sin objetivo
conserva la última orientación. R restablece la orientación hacia la derecha.
Los proyectiles de pistola miden 28 x 14 px y los de metralleta 14 x 6 px;
ambos viajan a 800 px/s. Escopeta utiliza perdigones de 8 x 8 px y energía,
proyectiles de 40 x 10 px. El giro es visual: las colisiones siguen alineadas
a los ejes. Si falta una textura, se dibuja un rectángulo de respaldo.

Ver [mecánicas de las armas nuevas](documentos/guia_escopeta_canon_energia.md)
y [integración de gráficos](documentos/guia_graficos_armas_nuevas.md).

### Estaciones, rarezas y cofres

Los objetos se distribuyen por zonas al explorar y conservan su estado durante
la ejecución. Cada estación permite una compra y queda apagada; cada cofre
entrega una recompensa y queda abierto.

Abrir una estación cobra chatarra y pausa el combate. Muestra tres ofertas
con rareza común, rara, épica o legendaria. Elegir una oferta incorpora el arma
con sus beneficios o mejora una ya poseída.

La primera arma nueva ocupa el segundo slot, que queda fijo. Después solo
puede reemplazarse la pistola del primero, con confirmación y pérdida de sus
bonos. Ese reemplazo fija también el primer slot. Cuando ambos están fijos,
solo se ofrecen mejoras de las armas actuales.

El precio de estación comienza en 10 y aumenta un 60% tras cada compra,
redondeando hacia arriba: 10, 16, 26, 42, 68… Cancelar devuelve el pago y
conserva las ofertas; no consume la estación ni aumenta el precio.

Los cofres entregan +20 de vida máxima y actual o +40 de experiencia.
Sus precios aumentan con un contador independiente: 15, 24, 39, 63…
No entregan armas ni mejoras de daño o cadencia.

Ver [adquisición y rarezas](documentos/guia_adquisicion_rareza_armas.md) para
probabilidades, porcentajes y topes, y [encuentros por zonas](documentos/encuentros_aleatorios.md)
para distribución y persistencia.

## Pruebas

Ejecutar los archivos `.bat` desde la raíz del proyecto. Las pruebas gráficas
abren brevemente una ventana de SFML.

| Archivo | Comprobaciones |
|---|---|
| `pruebas/probar_juego_proyectiles.bat` | Disparos simultáneos, cadencia, colisiones, recompensas y reinicio |
| `pruebas/probar_camara.bat` | Seguimiento, cobertura del suelo, coordenadas negativas, proyectiles y reinicio |
| `pruebas/probar_estacion_mejoras.bat` | Cobro, ofertas, adquisición, reemplazo, pausa, reembolso y conservación de compras |
| `pruebas/probar_inventario_armas.bat` | Slots, rarezas, bonificaciones, especiales y límites |
| `pruebas/probar_aspecto_armas.bat` | Carga de texturas, orientación y respaldo sin imágenes |
| `pruebas/probar_armas_nuevas.bat` | Abanico de escopeta, perforación, identidades, recargas y gráficos |
| `pruebas/probar_encuentros.bat` | Distribución de estaciones y cofres, interacción, precios y recompensas |

## Estructura

- `src`: implementaciones `.cpp`.
- `include`: encabezados `.h`.
- `recursos`: sprites, fuentes y sonidos.
- `datos`: carpeta prevista para archivos binarios; su persistencia sigue pendiente.
- `documentos`: decisiones y documentación del equipo.
- `pruebas`: pruebas de los módulos y de integración.
- `dependencias`: bibliotecas locales no incluidas en el repositorio.

`Arma` conserva estadísticas y controla la recarga. `Proyectil` administra
movimiento, daño y estado de cada disparo. `InventarioArmas` mantiene slots y
bonos; `EstacionMejoras`, ofertas y precios. `Juego` coordina combate,
interacciones y presentación. Las interfaces temporales de `Jugador` deberán
coordinarse con su responsable antes de reemplazarlas.
