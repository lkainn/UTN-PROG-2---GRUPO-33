# Guía de estudio: daño, recompensas e interfaz — Proyecto TPI

Archivos para seguir la explicación: [`Juego.h`](../include/Juego.h),
[`Juego.cpp`](../src/Juego.cpp), [`Enemigo.h`](../include/Enemigo.h),
[`Jugador.h`](../include/Jugador.h),
[`Jugador.cpp`](../src/Jugador.cpp),
[`SistemaColisiones.cpp`](../src/SistemaColisiones.cpp) y
[`TiposCompartidos.h`](../include/TiposCompartidos.h).

## 1. Quién hace cada cosa

| Parte | Responsabilidad en esta sesión |
|---|---|
| `Enemigo` | Informa su forma, daño de contacto y recompensa; recibe el daño del proyectil. |
| `Jugador` | Descuenta vida, guarda experiencia, nivel y chatarra. |
| `SistemaColisiones` | Responde si dos rectángulos se superponen o si dos puntos están dentro de un radio. |
| `Juego` | Coordina contactos, tiempos, drops, recolección y dibujo. |

`Recompensa` es una estructura compartida con dos valores: `experiencia` y
`chatarra`. `Enemigo::obtenerRecompensa()` devuelve esos valores, pero no los
entrega directamente al jugador.

## 2. Daño por contacto enemigo–jugador

En cada fotograma, `Juego::actualizarDanoContacto(segundos)` recorre los tres
enemigos. Solo considera a los vivos y pregunta a `SistemaColisiones` si el
rectángulo del enemigo intersecta el del jugador:

```cpp
if (!enemigosPrueba[i].estaVivo() ||
    !SistemaColisiones::hayInterseccion(
        formaJugador, enemigosPrueba[i].obtenerFormaColision())) {
    continue;
}
```

`continue` pasa al siguiente enemigo cuando el actual está muerto o no toca al
jugador. Cuando sí hay contacto, `Juego` consulta el daño propio de ese enemigo
y llama a `jugador.recibirDano(...)`. `Jugador` es quien modifica su vida y
marca `_vivo = false` si esta llega a cero.

### Por qué se calcula el daño realmente recibido

El número flotante usa la diferencia entre la vida anterior y la posterior:

```cpp
const int vidaAnterior = jugador.getVida();
jugador.recibirDano(enemigosPrueba[i].obtenerDano());
const int danoRecibido = vidaAnterior - jugador.getVida();
```

Por ejemplo, si quedan `5` puntos de vida y el enemigo hace `10` de daño, la
vida termina en `0`. El indicador muestra `-5`, que es lo que efectivamente se
perdió, y no `-10`. Si `danoRecibido > 0`, se agrega un `IndicadorDanio` con
cantidad, posición y tiempo restante.

### Pausa entre golpes

Después de un contacto efectivo se fija
`tiempoRestanteDanoContacto = 0.75f`. En cada fotograma se le resta `segundos`
hasta llegar a cero. Mientras sea positivo, no se aplica otro golpe. Es un
temporizador **global del jugador**, no uno distinto para cada enemigo. Además,
el `break` termina el recorrido tras procesar un contacto, de modo que varios
enemigos superpuestos no descuenten vida simultáneamente en ese fotograma.

La forma rectangular proviene de `getFormaColision()` y
`obtenerFormaColision()`. `SistemaColisiones` comprueba que las dimensiones sean
positivas y los valores finitos antes de evaluar la intersección.

## 3. Indicador flotante del daño

`indicadoresDanio` es un `std::vector<IndicadorDanio>`. Cada golpe puede crear
un indicador y varios pueden coexistir, por lo que su cantidad no es fija.
`Juego::actualizarIndicadoresDanio()` mueve cada indicador hacia arriba, resta
su tiempo de vida y elimina los vencidos con `erase` + `std::remove_if`.

Al dibujarlo, el tiempo restante se transforma en transparencia: el texto rojo
`-cantidad` se desvanece durante `0,8` segundos. La fuente se intenta cargar
desde `recursos/Tuffy.ttf`. Si falta, el juego conserva el daño y simplemente
omite los textos; esta misma condición afecta los indicadores de recursos y la
interfaz de nivel, experiencia y chatarra.

## 4. Muerte del enemigo y generación de recursos

En `Juego::actualizarProyectil()`, un impacto aplica el daño del proyectil al
enemigo y desactiva ese proyectil. Si con ese golpe el enemigo dejó de estar
vivo, `Juego` ejecuta una sola vez:

```cpp
generarRecompensa(enemigosPrueba[i]);
tiemposReaparicion[i] = SEGUNDOS_REAPARICION;
```

`generarRecompensa()` consulta `obtenerRecompensa()`, calcula el centro del
enemigo a partir de su rectángulo y crea hasta dos recursos en el suelo: uno de
experiencia y otro de chatarra. Cada recurso guarda su tipo, cantidad y
posición. Los dos cuadrados se separan horizontalmente para que se vean.

El drop no aumenta inmediatamente los contadores del jugador. Primero queda
en `recursosSuelo`, que es un `std::vector<RecursoSuelo>`. Así el jugador debe
acercarse a recogerlo mientras los demás enemigos siguen activos.

## 5. Recolección por proximidad

`Juego::actualizarRecursos()` recorre los recursos del suelo y compara la
posición del jugador con la de cada recurso mediante
`SistemaColisiones::estaDentroDeRadio(...)`. El radio actual es de `90`
píxeles. La comprobación compara distancias **al cuadrado**:

```cpp
dx * dx + dy * dy <= radio * radio
```

Así se evita calcular una raíz cuadrada en cada comprobación. Si el jugador
está fuera del radio, el iterador avanza. Si entra en el radio, se llama a
`jugador.ganarExperiencia(cantidad)` o
`jugador.ganarChatarra(cantidad)`, se crea un indicador flotante y el recurso
se borra del vector. Al borrarlo ya no puede recogerse por segunda vez.

El recorrido usa un iterador y guarda el valor devuelto por `erase`:

```cpp
recurso = recursosSuelo.erase(recurso);
```

Ese valor señala el siguiente elemento válido. Incrementar el iterador viejo
después de borrar sería incorrecto porque quedó invalidado. Si el jugador está
muerto, la función no recoge recursos.

## 6. Experiencia, nivel y chatarra del jugador

`Jugador::ganarExperiencia()` suma la cantidad y llama a
`verificarSubidaNivel()`. El método usa `while`: si una sola recolección alcanza
para varios niveles, procesa todos. En cada subida resta la experiencia exigida,
aumenta el nivel, sube en `50` la exigencia del próximo nivel, aumenta en `20`
la vida máxima y restaura la vida a ese nuevo máximo.

`getExperiencia()` devuelve la experiencia **que queda hacia el próximo
nivel**, pues la experiencia requerida se descuenta al subir. `getChatarra()`
devuelve la chatarra acumulada en el estado actual; recogerla no la gasta ni
activa todavía un sistema de compras.

## 7. Qué se dibuja y qué se guarda

`dibujarRecursos()` representa experiencia con un cuadrado celeste y chatarra
con uno amarillo. Al recogerlos, `dibujarIndicadoresRecurso()` muestra
`+cantidad EXP` o `+cantidad CHATARRA` durante un segundo, con desplazamiento
hacia arriba y transparencia gradual.

`dibujarInterfaz()` muestra en la esquina superior izquierda el nivel, la
experiencia actual y la chatarra. Es una lectura del estado de `Jugador` cada
fotograma; no mantiene contadores duplicados en `Juego`.

### Por qué se usa `std::vector`

Los recursos y los indicadores son datos temporales de la partida. No sabemos
de antemano cuántos habrá: cada enemigo muerto puede dejar hasta dos recursos,
cada recolección genera un indicador y varios golpes pueden dejar indicadores
de daño activos a la vez. Durante el juego se agregan y eliminan elementos.
`std::vector` es idóneo para esa cantidad variable: almacena objetos del mismo
tipo y ajusta su capacidad cuando necesita espacio.

En este código, `recursosSuelo.push_back(...)` agrega un drop,
`recursosSuelo.erase(recurso)` lo elimina después de recogerlo y `clear()`
quita los que quedan al reiniciar. Los vectores de indicadores usan las mismas
ideas para crear textos y retirar los que vencieron. El tamaño lógico del
vector acompaña la cantidad real de objetos presentes.

También resulta más intuitivo para este caso: podemos expresar directamente
«agregar», «recorrer» y «quitar» sin llevar un arreglo manual, una variable con
la cantidad de posiciones ocupadas y código para agrandarlo cuando se llene.
Con `new[]` y `delete[]` tendríamos que reservar, copiar al crecer y liberar
la memoria correctamente en cada camino. `std::vector` administra esa memoria
de forma automática y la libera al destruirse el contenedor; así se reducen
los riesgos de fugas y dobles liberaciones. `clear()` elimina los elementos,
aunque el vector puede conservar memoria reservada para reutilizarla.

Esto no significa usar `vector` para todo. Los tres puestos de enemigos
mantienen siempre el mismo tamaño, por lo que `std::array` representa mejor
esa parte del juego. La elección depende de si la cantidad puede variar o es
fija, no de usar memoria dinámica por obligación.

Al presionar `R`, `Juego` limpia recursos e indicadores. El método actual
`Jugador::reiniciar()` restaura posición y vida, pero **conserva** nivel,
experiencia y chatarra; la interfaz continúa mostrando esos valores.
