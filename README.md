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

El perfil, las mejoras permanentes, el catalogo de enemigos y los resultados
se guardan en `datos`. Esa carpeta se comparte al ejecutar desde la raiz y
desde `bin/Debug`. U compra una mejora permanente de vida por 10 de chatarra;
N reinicia el perfil y las mejoras sin borrar el historial de partidas.
Al ganar o perder se guarda un unico resultado. Los detalles y las pruebas
estan en `documentos/integracion_persistencia.md`.

Remache usa sprites pixel art con desgaste industrial y ocho direcciones de
movimiento. El cuerpo y ambos brazos se orientan al objetivo antes de apuntar
las armas desde sus pinzas. Los pasos alternados avanzan con la distancia
recorrida a un ritmo aproximado de dos ciclos por segundo a velocidad normal.
El movimiento diagonal tiene la misma rapidez que el cardinal.
La apariencia conserva una altura de
144 px y la colision sigue siendo de 48 x 48. Los atlas estan en
`recursos/personajes/remache_atlas`; el formato y el respaldo se describen en
`recursos/personajes/diseno.md`. La prueba visual se ejecuta con
`pruebas/probar_aspecto_jugador.bat`.

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

### Menú inicial

Al abrir el juego aparece el menú con Jugar, Controles y Salir. Las flechas
arriba y abajo seleccionan una opción y Enter la confirma. Desde Controles,
Enter o Escape vuelven al menú. El combate comienza al elegir Jugar.
Escape desde el menú principal cierra la ventana.

El menu usa la fuente existente y formas de SFML. El perfil y las mejoras
permanentes se recuperan al comenzar una partida.

Durante la partida, Escape abre el menú de pausa con Continuar y Salir.
Flechas y Enter seleccionan la acción; Escape también permite continuar.
La pausa conserva la partida y detiene movimiento, daño, disparos y oleadas.
Si una estación está abierta, Escape cancela primero la compra y devuelve
el pago; otro Escape abre la pausa.

La prueba del menú se ejecuta con pruebas/probar_menu_principal.bat.

### Controles

| Tecla | Acción |
|---|---|
| WASD | Mover al jugador |
| F | Interactuar con la estación o cofre disponible más cercano, dentro de 100 px |
| 1 / 2 / 3 | Elegir una oferta de la estación |
| 1 / 2 en la confirmación | Confirmar el reemplazo de pistola / volver a las ofertas |
| Escape | Cancelar una compra pendiente y devolver el pago; durante el juego, abrir pausa; en pausa, continuar |
| R | Reiniciar la prueba conservando inventario, mejoras, precios y encuentros usados |
| K | Recibir 20 de daño para pruebas, fuera del menú |
| E | Obtener 40 de experiencia para pruebas, fuera del menú |
| U | Comprar una mejora permanente de vida por 10 de chatarra |
| N | Reiniciar perfil y mejoras, conservando el historial de partidas |

También se admiten las teclas numéricas del teclado numérico. R restaura la
vida, limpia disparos y recursos del suelo, reinicia las recargas y selecciona
el slot 1 actual. No recupera una pistola descartada.

### Armas y gráficos

Se inicia con pistola en el primer slot y el segundo vacío. Las demás armas
se adquieren en estaciones junto con los beneficios de la oferta elegida.
Las dos armas equipadas disparan automáticamente, cada una con su alcance,
orientación y recarga. Las balas nacen en la boca del arma dibujada, incluso
al girar hacia la izquierda.

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
se dibuja desde la pinza y acompana la orientacion del personaje hacia el
objetivo automatico. Sin objetivo sigue la vista del personaje.
Los proyectiles de pistola miden 28 x 14 px y los de metralleta 14 x 6 px;
ambos viajan a 800 px/s. Escopeta utiliza perdigones de 8 x 8 px y energía,
proyectiles de 40 x 10 px. El giro es visual: las colisiones siguen alineadas
a los ejes. Si falta una textura, se dibuja un rectángulo de respaldo.

Ver [mecánicas de las armas nuevas](documentos/guia_escopeta_canon_energia.md)
y [integración de gráficos](documentos/guia_graficos_armas_nuevas.md).

### Graficos de encuentros industriales

Los cofres se representan en pantalla como contenedores de suministros de
acero, con estados cerrado y vacio. Las estaciones utilizan terminales con
pantalla verde que se apaga tras la compra. Ambos mantienen sus reglas de
interaccion, precios y recompensas.

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

### Partida completa

El jefe chatarrero aparece a los cinco minutos, con aviso durante los cinco
segundos previos y color naranja. Mientras vive, los enemigos comunes aparecen
mas espaciados. La interfaz muestra bajas, puntaje y estado del jefe.

La muerte del jugador termina en derrota y la del jefe en victoria. Si ambos
mueren en el mismo fotograma, tiene prioridad la derrota. La pantalla final
muestra duracion, bajas, chatarra recogida, nivel y puntaje. Enter comienza una
partida nueva; Escape vuelve al menu principal. El resultado detiene el combate.

El puntaje suma 10 por baja, 2 por unidad de chatarra recogida y 5000 por victoria.
La chatarra gastada sigue contando como recogida; los reembolsos no suman puntos.
La duracion excluye las pausas y los menus de estaciones.

Una partida nueva recupera el perfil y las mejoras permanentes, y restablece
inventario, beneficios temporales, encuentros y precios.
R sigue siendo un reinicio de prueba que conserva las compras y el progreso.
El resultado se guarda una sola vez en Partidas.dat y actualiza el record
de puntaje del perfil. La prueba se ejecuta con
pruebas/probar_partida_integrada.bat.

Ver la [guía de integración de partida y disparo de dos armas](documentos/guia_partida_integrada_y_disparo_doble.md)
para el ciclo de juego, reinicio completo, origen de las balas y ejemplos de código.
