# Oleadas de enemigos y dificultad progresiva

Fecha de referencia: 5 de octubre de 2026.

## Objetivo

Este incremento concentra en una sola clase la aparicion, el movimiento y la
eliminacion de los enemigos de una partida, y hace que la dificultad aumente
con el tiempo transcurrido.

Reemplaza el esquema temporal que mantenia `Juego`: un arreglo fijo de nueve
enemigos con posiciones hardcodeadas y un temporizador de reaparicion por cada
uno.

No incluye todavia el jefe final, los contadores de la partida ni la
persistencia en archivos binarios.

## Responsabilidades

### Enemigo

- Conserva sus estadisticas y su estado temporal en memoria.
- Usa `x` e `y` como esquina superior izquierda.
- Persigue una posicion objetivo con movimiento dependiente de `deltaTime`.
- Expone su hitbox, su dano de contacto, su recompensa y su id de variante.
- No depende de SFML ni conoce la clase Jugador.

### GestorOleadas

- Conserva los enemigos de la partida en un `std::vector<Enemigo>`.
- Decide cuando aparece un enemigo nuevo, en que posicion y de que variante.
- Mantiene el reloj de partida del que depende toda la dificultad.
- Mueve a todos los enemigos y elimina a los que dejaron de estar vivos.
- Informa el estado de la partida para la interfaz: tiempo, oleada, cantidad
  de enemigos y techo vigente.
- No depende de SFML: trabaja con `Posicion` y con segundos.

### Juego

- Conserva una instancia de `GestorOleadas` por valor.
- Invoca `actualizar()` una vez por fotograma con la posicion del jugador.
- Recorre `obtenerEnemigos()` para dibujar y para resolver colisiones.
- Elige el color de cada enemigo segun `obtenerIdEnemigo()`.
- Invoca `reiniciar()` cuando comienza una partida nueva.

## Interfaz publica

```cpp
void actualizar(Posicion posicionJugador, float dt);
const std::vector<Enemigo>& obtenerEnemigos() const;
void reiniciar();
std::size_t cantidadEnemigos() const;
void danarATodos(int cantidad);          // solo para pruebas del modulo

float tiempoDePartida() const;
int oleadaActual() const;
float intervaloActual() const;
std::size_t maximoVivosActual() const;
```

`obtenerEnemigos()` devuelve una referencia constante: los enemigos se
modifican unicamente desde `actualizar()`. Quien dibuja o resuelve colisiones
solo lee.

`danarATodos()` existe para demostrar el modulo sin proyectiles. En el juego el
dano lo aplica quien detecta el impacto, llamando a `Enemigo::recibirDanio()`.

## Flujo de actualizacion

En cada fotograma, `actualizar()` realiza estas operaciones en este orden:

1. Suma el `deltaTime` al reloj de partida.
2. Suma el `deltaTime` al temporizador de aparicion. Si alcanzo el intervalo
   vigente, lo reinicia y, si la cantidad de enemigos es menor que el techo
   vigente, agrega un enemigo nuevo.
3. Recorre el vector y actualiza a cada enemigo con su objetivo corregido.
4. Elimina del vector a los enemigos que dejaron de estar vivos.

El orden no es intercambiable. La eliminacion se realiza en un recorrido
aparte: borrar elementos durante el recorrido invalida las referencias y puede
omitir enemigos. Se usa `erase` junto con `remove_if`, el mismo criterio
aplicado a los proyectiles simultaneos.

El temporizador funciona por acumulacion de `deltaTime` y no mediante esperas,
que detendrian el ciclo completo del juego.

## Posicion de aparicion

Los enemigos aparecen sobre un anillo de 700 pixeles alrededor del jugador, en
un angulo sorteado entre cero y una vuelta completa:

```cpp
aparicion.x = centro.x + std::cos(angulo) * DISTANCIA_APARICION;
aparicion.y = centro.y + std::sin(angulo) * DISTANCIA_APARICION;
```

Se descarto usar los bordes de la ventana porque la camara de seguimiento hizo
que dejaran de coincidir con los limites del mundo, que en el prototipo actual
no tiene borde fisico. El anillo es independiente del tamano de ventana y de la
posicion del jugador en el mundo.

La distancia debe superar la mitad de la diagonal de la vista para que ningun
enemigo aparezca a la vista del jugador.

El anillo ubica la esquina superior izquierda del enemigo. El centro queda a lo
sumo a media diagonal de su forma del anillo exacto: para 56 x 56 pixeles, unos
40 sobre 700. La diferencia no es perceptible y no se corrige.

## Correccion del centro

El gestor entrega a cada enemigo un objetivo ya corregido:

```cpp
objetivo.x = posicionJugador.x - forma.ancho / 2.0f;
objetivo.y = posicionJugador.y - forma.alto / 2.0f;
```

Sin esta conversion el enemigo perseguiria con su esquina superior izquierda y
quedaria desplazado media forma respecto del jugador. La correccion queda dentro
del gestor, de modo que `Juego` puede eliminar la que realiza actualmente.

## Oleadas

Una oleada es un tramo de 30 segundos. Su numero se obtiene dividiendo el reloj
de partida por esa duracion. No existe condicion de cierre: no hace falta
eliminar a todos los enemigos para avanzar.

Se eligio este criterio porque con oleadas cerradas un enemigo alejado del
jugador, o incapaz de alcanzarlo, puede impedir el avance de forma indefinida.

La numeracion sirve para mostrarla en pantalla y para registrarla en la partida.
La dificultad no depende de ella: es continua y no salta al cambiar de tramo.

## Curva de dificultad

Una sola magnitud gobierna el balance. Avanza linealmente de 0 a 1 entre el
inicio de la partida y el segundo 300, y despues se mantiene en 1.

```cpp
float GestorOleadas::progresoDificultad() const {
    if (tiempoPartida >= SEGUNDOS_DIFICULTAD_MAXIMA) { return 1.0f; }
    return tiempoPartida / SEGUNDOS_DIFICULTAD_MAXIMA;
}
```

El intervalo de aparicion y el techo de enemigos vivos se interpolan con ese
valor. El intervalo desciende y el techo asciende.

Comportamiento verificado, simulando seis minutos sin eliminar enemigos:

```
 min | oleada | intervalo | tope | vivos | basicos rapidos resist
   0 |      1 |      1.50 |   15 |     0 |       0       0      0
   1 |      3 |      1.28 |   24 |    23 |      23       0      0
   2 |      5 |      1.06 |   33 |    32 |      26       6      0
   3 |      7 |      0.84 |   42 |    41 |      29      12      0
   4 |      9 |      0.62 |   51 |    50 |      33      15      2
   5 |     11 |      0.40 |   60 |    60 |      36      20      4
```

El techo de enemigos vivos es imprescindible: sin el, el vector crece sin
limite y cada fotograma debe mover y dibujar a todos los enemigos acumulados.

El temporizador de aparicion continua funcionando cuando el techo esta
completo. Lo que se descarta es esa aparicion puntual, no el ciclo.

## Variantes

Las tres variantes son la misma clase `Enemigo` construida con distintos
valores. No se emplea herencia.

| Variante | id | Tamano | Vida | Velocidad | Dano | Experiencia | Chatarra |
|---|---|---|---|---|---|---|---|
| BASICO | 1 | 56 x 56 | 100 | 120 px/s | 10 | 5 | 3 |
| RAPIDO | 2 | 40 x 40 | 60 | 220 px/s | 8 | 7 | 2 |
| RESISTENTE | 3 | 72 x 72 | 250 | 70 px/s | 15 | 12 | 6 |

Se habilitan segun el reloj de partida y entre las disponibles se sortea con
igual probabilidad:

| Desde el segundo | Variantes disponibles |
|---|---|
| 0 | BASICO |
| 60 | BASICO, RAPIDO |
| 180 | BASICO, RAPIDO, RESISTENTE |

De esta manera la progresion no consiste unicamente en una mayor cantidad de
enemigos: tambien cambia el tipo de amenaza.

Los id 1, 2 y 3 corresponden a las filas previstas de `Enemigos.dat`. Al
implementar la persistencia, la funcion `crearEnemigo()` reemplaza sus valores
escritos en codigo por la lectura del registro correspondiente, sin que cambie
el resto del modulo.

## Constantes de balance

Reunidas al comienzo de `src/GestorOleadas.cpp`:

| Constante | Valor | Significado |
|---|---|---|
| `DISTANCIA_APARICION` | 700 | radio del anillo, en pixeles |
| `SEGUNDOS_POR_OLEADA` | 30 | duracion de cada tramo |
| `SEGUNDOS_DIFICULTAD_MAXIMA` | 300 | cuando deja de aumentar |
| `INTERVALO_APARICION_INICIAL` | 1.5 | segundos entre apariciones al inicio |
| `INTERVALO_APARICION_MINIMO` | 0.4 | segundos entre apariciones al final |
| `MAXIMO_VIVOS_INICIAL` | 15 | techo al inicio |
| `MAXIMO_VIVOS_FINAL` | 60 | techo al final |
| `SEGUNDOS_APARECE_RAPIDO` | 60 | habilitacion de la variante rapida |
| `SEGUNDOS_APARECE_RESISTENTE` | 180 | habilitacion de la variante resistente |

Todas corresponden a balance y pueden ajustarse sin modificar la logica.

## Generacion aleatoria

Se emplea `std::mt19937` con `uniform_real_distribution` para el angulo y
`uniform_int_distribution` para la variante. Se descarto `rand()` por su rango
reducido y por la distribucion despareja que produce al escalarlo.

La semilla es fija mientras se prueba el modulo, para que las partidas sean
reproducibles ante un comportamiento inesperado. Se reemplaza por
`std::random_device` cuando el comportamiento quede definido.

## Prueba del modulo

`pruebas/prueba_enemigos.cpp`, compilada con `compilar_pruebas.bat`, genera
`bin/Debug/PruebaEnemigos.exe`. Tiene su propio `main` y su propia ventana: no
depende de `Juego`, de `Jugador` ni de `Proyectil`.

El objetivo es una `Posicion` que se desplaza con las flechas y reemplaza al
jugador. Un circulo gris indica el anillo de aparicion. El titulo de la ventana
informa oleada, tiempo, cantidad de enemigos sobre el techo vigente e intervalo
actual, y se actualiza cuatro veces por segundo.

- Flechas: desplazar el objetivo.
- Espacio: aplicar dano a todos los enemigos vivos.
- R: reiniciar la partida.
- Escape: cerrar.

El `deltaTime` se limita a 0.05 segundos, con el mismo criterio que `Juego`.

## Verificaciones realizadas

- Ritmo de aparicion: 6 enemigos en los primeros 10 segundos, con el intervalo
  inicial de 1.5 segundos.
- Techo de vivos: tras 120 segundos la cantidad se mantiene en el techo
  vigente y no lo supera.
- Distancia de aparicion: 12 apariciones medidas en el fotograma siguiente a
  su creacion, entre 660 y 737 pixeles del jugador, consistente con un anillo
  de 700 y una forma de 56 x 56.
- Persecucion: tras 20 segundos el enemigo mas cercano se encuentra a 39
  pixeles del objetivo, habiendo aparecido a 700.
- Eliminacion: luego de aplicar dano suficiente, el vector queda vacio.
- Reinicio: `reiniciar()` deja la oleada en 1, el vector vacio y el intervalo
  en su valor inicial.
- Variantes: la composicion simulada confirma que los rapidos aparecen a
  partir del segundo 60 y los resistentes a partir del 180.
- Compilacion sin advertencias con `-Wall -Wextra` en C++17.

## Limitaciones y trabajo pendiente

- No existe el jefe final. Esta previsto como una cuarta variante con
  aparicion unica, cuya eliminacion determina la victoria.
- El gestor no lleva contadores de enemigos eliminados ni de recompensas
  entregadas. Corresponden a la clase `Partida`, aun no implementada.
- No hay condicion de victoria ni de derrota administrada por el modulo.
- No se leen las variantes desde `Enemigos.dat` ni se registran resultados en
  `Partidas.dat`.
- El gestor todavia no esta integrado en `Juego`, que conserva su arreglo fijo
  de nueve enemigos.
- En el esquema temporal vigente de `Juego`, el arreglo `TIPOS_ENEMIGOS` tiene
  tres elementos y se indexa con el indice del enemigo, que alcanza el valor
  ocho. Los indices 3 a 8 leen fuera del arreglo: `std::array::operator[]` no
  verifica el rango y devuelve contenido indeterminado. La integracion del
  gestor elimina ese acceso.
