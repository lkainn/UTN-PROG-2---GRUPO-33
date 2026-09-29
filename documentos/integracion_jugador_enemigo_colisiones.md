# Integracion de jugador, enemigo, proyectil y colisiones

Fecha de referencia: 24 de septiembre de 2026.

## Objetivo

Este incremento conecta las clases `Jugador`, `Enemigo`, `Proyectil` y
`SistemaColisiones` en una demostracion jugable. Permite mover al jugador,
disparar contra un enemigo que lo persigue, aplicar dano mediante hitboxes
rectangulares, eliminar al enemigo y reiniciar la prueba.

El incremento sigue siendo una demostracion tecnica. No representa todavia el
sistema definitivo de combate, enemigos u oleadas.

## Responsabilidades

### Jugador

- Mantiene posicion, velocidad, vida, nivel, experiencia y chatarra.
- Procesa movimiento mediante WASD o flechas.
- Corrige su posicion para permanecer dentro de la ventana.
- Expone una `FormaColision` rectangular.
- Conserva dentro de la clase su figura temporal de SFML.

### Enemigo

- Mantiene sus estadisticas y su estado temporal en memoria.
- Usa `x` e `y` como esquina superior izquierda.
- Persigue una posicion objetivo con movimiento dependiente de `deltaTime`.
- Expone su hitbox mediante `obtenerFormaColision()`.
- Recibe dano y deja de estar vivo cuando su vida llega a cero.
- No depende de SFML. Su representacion grafica temporal pertenece a `Juego`.

### Proyectil

- Mantiene posicion, velocidad, dimensiones, dano y estado activo.
- Actualiza su posicion utilizando segundos transcurridos.
- Expone una `FormaColision` independiente de SFML.
- Se desactiva al impactar o al salir del escenario.

### SistemaColisiones

- Compara rectangulos sin conocer las clases concretas.
- `hayInterseccion()` se usa para el impacto proyectil-enemigo.
- `estaDentroDe()` se usa para detectar la salida del proyectil del escenario.
- Rechaza formas con dimensiones no positivas o valores no finitos.

### Juego

- Coordina entrada, actualizacion, colisiones y dibujo.
- Conserva las instancias de `Jugador`, `Enemigo` y `Proyectil`.
- Sincroniza las figuras temporales de SFML con la posicion logica del enemigo
  y del proyectil.
- Aplica al enemigo el dano informado por el proyectil.
- Reinicia el estado completo de la demostracion.

## Decision sobre el renderizado

Las figuras de SFML del enemigo y del proyectil se mantienen temporalmente en
`Juego` para esta demostracion. Mas adelante se incorporara una clase especifica
de renderizado, cuyo nombre y diseño se definiran cuando se integren los sprites
y las animaciones definitivas.

Esa clase se ocupara de crear, sincronizar y dibujar las representaciones
visuales a partir del estado logico de las entidades. `Jugador`, `Enemigo` y
`Proyectil` conservaran como responsabilidad principal su estado, comportamiento
y forma de colision. Al realizar esa separacion tambien se revisara la figura de
SFML que actualmente se encuentra dentro de `Jugador` para aplicar un criterio
uniforme a todas las entidades.

## Flujo de actualizacion

En cada fotograma se realizan estas operaciones:

1. Se calcula el tiempo transcurrido y se limita a `0.05` segundos para reducir
   saltos grandes después de una pausa de la ventana.
2. El jugador procesa la entrada y valida los limites de pantalla.
3. Si el enemigo esta vivo, se calcula como objetivo la esquina superior
   izquierda que coloca su centro sobre el centro del jugador.
4. El enemigo avanza hacia ese objetivo y su rectangulo grafico se sincroniza
   con la nueva posicion.
5. Si existe un proyectil activo, se actualiza su posicion.
6. Se comparan la forma del proyectil y la forma del enemigo.
7. Si existe impacto, el enemigo recibe el dano y el proyectil se desactiva.
8. Si no hubo impacto y el proyectil salio del escenario, se desactiva.

La desactivacion inmediata evita que el mismo proyectil produzca dano en mas de
un fotograma.

## Convencion de posiciones e hitboxes

`Enemigo`, `Proyectil` y `FormaColision` interpretan `x` e `y` como la esquina
superior izquierda. El rectangulo de SFML del enemigo conserva el origen
predeterminado en esa misma esquina y recibe exactamente la posicion devuelta
por `Enemigo::obtenerPosicion()`.

Por lo tanto, el rectangulo visible y el hitbox del enemigo tienen la misma
posicion y las mismas dimensiones: `56 x 56` pixeles.

El jugador trabaja internamente con una posicion central. Al calcular el
objetivo del enemigo se resta la mitad de su ancho y alto:

```cpp
const Posicion objetivoEnemigo{
    posicionJugador.x - formaEnemigo.ancho / 2.0f,
    posicionJugador.y - formaEnemigo.alto / 2.0f
};
```

Esta conversion permite que el centro del enemigo persiga al centro del
jugador sin desplazar su hitbox.

## Impacto, muerte y reinicio

La demostracion utiliza estos valores temporales:

- Vida del enemigo: `100`.
- Dano por proyectil: `25`.
- Impactos necesarios para eliminarlo: `4`.
- Velocidad del enemigo: `150` pixeles por segundo.
- Velocidad horizontal del proyectil: `800` pixeles por segundo.

Cuando la vida del enemigo llega a cero, deja de actualizarse y dibujarse. La
tecla `R` reconstruye la instancia con sus valores iniciales, vuelve a
sincronizar la figura, desactiva cualquier proyectil, restaura el titulo de la
ventana y reinicia el reloj de fotogramas.

La reasignacion es segura con la implementacion actual de `Enemigo` porque sus
atributos son valores simples y un arreglo fijo `char[30]`. Esta decision debe
revisarse si en el futuro la clase administra memoria dinamica u otros recursos.

## Controles de la demostracion

- `WASD` o flechas: mover al jugador.
- `Espacio`: disparar un proyectil hacia la derecha.
- `R`: reiniciar enemigo, proyectil y reloj de la prueba.
- `K`: aplicar 20 puntos de dano al jugador para pruebas.
- `E`: otorgar 40 puntos de experiencia al jugador para pruebas.
- `Escape`: cerrar la ventana.

## Archivos involucrados

- `include/Juego.h` y `src/Juego.cpp`: coordinacion de la demostracion.
- `include/Jugador.h` y `src/Jugador.cpp`: movimiento y estado del jugador.
- `include/Enemigo.h` y `src/Enemigo.cpp`: persecucion y vida del enemigo.
- `include/Proyectil.h` y `src/Proyectil.cpp`: movimiento y dano del proyectil.
- `include/SistemaColisiones.h` y `src/SistemaColisiones.cpp`: intersecciones y
  limites.
- `include/TiposCompartidos.h`: `Posicion`, `FormaColision` y `Recompensa`.
- `ProyectoTPI.cbp` y `compilar_debug.bat`: incorporacion de las clases a la
  compilacion.

## Limitaciones y trabajo pendiente

- Las operaciones de dano, experiencia y chatarra del jugador conservan por
  ahora el comportamiento original frente a cantidades negativas. Su autor
  revisara esas validaciones por separado.
- `Jugador::cargarEstado()` conserva por ahora la implementacion aportada por
  su autor y no reconstruye valores derivados como vida maxima o umbral de
  experiencia.
- Solo existe un enemigo y un proyectil activo a la vez.
- El disparo es manual y se desplaza solamente hacia la derecha.
