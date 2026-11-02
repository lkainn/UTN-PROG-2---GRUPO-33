# Guía de estudio del TPI — El Último Recolector

Guía integral del proyecto: cómo está armado, por qué cada decisión, y una
batería de ejercicios de modificación para la defensa.

Las guías por módulo (`guia_oleadas_enemigos.md`, `guia_estudio_arma.md`,
`guia_adquisicion_rareza_armas.md`, etc.) entran en más detalle en cada tema.
Esta las conecta y agrega la práctica de modificar.

Lucas armó además un **resumen función por función** de los 21 archivos del
proyecto (`resumen_local_proyecto/`, fuera del repositorio). Sirve para la
consulta puntual —«¿qué hace exactamente esta función?»—; esta guía sirve para
entender cómo se conecta todo y para practicar modificaciones. Conviene tener
las dos a mano.

---

# PARTE 1 — Cómo está armado el juego

## 1. El juego en una frase

Videojuego de supervivencia tipo *Vampire Survivors*: un robot se mueve por un
escenario industrial, las armas disparan automáticamente, y hay que sobrevivir
oleadas de máquinas hostiles hasta derrotar al jefe final.

- **Victoria**: derrotar al jefe (aparece en el minuto 5).
- **Derrota**: la vida del jugador llega a cero.

Tres módulos, uno por integrante:

| Módulo | Responsable | Clases |
|--------|-------------|--------|
| Jugador y progresión | Nicolás| `Jugador`, `ArchivoJugadores`, `ArchivoMejoras` |
| Enemigos y partida   | Ramiro | `Enemigo`, `GestorOleadas`, `Partida`, `ArchivoEnemigos`, `ArchivoPartidas` |
| Combate e integración| Lucas  | `Arma`, `Proyectil`, `SistemaColisiones`, `InventarioArmas`, `EstacionMejoras`, `Escenario`, `EncuentroAleatorio`, `Juego`, los `Aspecto*` |

---

## 2. El game loop: la base de todo

Todo el juego es un bucle que se repite 60 veces por segundo. En
`src/Juego.cpp`:

```cpp
void Juego::ejecutar() {
    while (ventana.isOpen()) {
        procesarEventos();   // que apretó el usuario
        actualizar();        // mover todo un poquito
        dibujar();           // mostrar cómo quedó
    }
}
```

Cada vuelta se llama **frame** (o fotograma). La ilusión de movimiento sale de
repetir eso muy rápido.

> **Si entendés esto, entendés el proyecto.** Cada módulo aporta su parte
> dentro de `actualizar()`, y `Juego` decide en qué orden.

### El `dt` (delta time)

Es cuántos **segundos** pasaron desde el frame anterior. A 60 FPS vale ~0.016.

```cpp
const float segundos = std::min(relojFotogramas.restart().asSeconds(), 0.05f);
```

Sirve para medir las velocidades **por segundo** en vez de por frame:

```cpp
x = x + 2;            // mal: 2 píxeles por frame → depende del framerate
x = x + 120 * dt;     // bien: 120 píxeles por segundo → igual en toda máquina
```

| FPS | dt | avanza por frame | en 1 segundo |
|---|---|---|---|
| 60 | 0.016 | 1.92 px | ~120 px |
| 30 | 0.033 | 4 px | ~120 px |

**El `std::min(..., 0.05f)` es importante:** si se arrastra o minimiza la
ventana, el frame siguiente tendría un `dt` enorme y todo se teletransportaría
—un enemigo podría atravesar al jugador sin que se detecte la colisión. El tope
lo evita.

### El orden del frame (`Juego::actualizar`)

```
1.  ¿el estado es "Jugando"? si no, salir (menú, controles, ranking,
    pausa o resultado: son los seis estados de EstadoJuego)
2.  ¿el menú de la estación está abierto? si sí, salir (pausa el combate)
3.  ¿el jugador está vivo? si no, cerrar la partida
4.  actualizarArmas          → avanza el enfriamiento de cada arma
5.  jugador.procesarEntrada  → WASD / flechas
6.  jugador.actualizar
7.  aspectoJugador, escenario (cámara)
8.  actualizarEncuentros     → cofres y estaciones
9.  actualizarEnemigos       → gestor: genera, mueve, limpia
10. actualizarDanoContacto   → enemigo que toca al jugador
11. actualizarIndicadoresDanio
12. actualizarProyectiles    → mueve, colisiona, aplica daño
13. actualizarRecursos       → experiencia y chatarra en el suelo
14. actualizarPartida        → contadores y desenlace
15. actualizarDisparoAutomatico
```

**El orden no es casual.** Dos ejemplos:

- Los proyectiles (12) se actualizan **después** de los enemigos (9), así
  impactan sobre la posición nueva y no sobre la del frame anterior.
- `actualizarPartida` (14) va **después** de los proyectiles, así las muertes de
  este frame se cuentan en este frame.

---

## 3. Mapa de archivos

### Base compartida

| Archivo | Qué es |
|---|---|
| `TiposCompartidos.h` | `Posicion`, `FormaColision`, `Recompensa` |
| `Configuracion.h` | ancho, alto y límite de FPS |
| `CarpetaDatos.h` | resuelve dónde está la carpeta `datos/` |
| `main.cpp` | crea `Juego` y llama a `ejecutar()`. Tres líneas. |

### Entidades (lógica pura, sin SFML)

| Archivo | Qué hace |
|---|---|
| `Enemigo` | persigue una posición, recibe daño, entrega recompensa |
| `Proyectil` | se mueve, registra impactos, se desactiva |
| `Arma` | enfriamiento, alcance, dirección del disparo |
| `SistemaColisiones` | compara rectángulos. Métodos `static`, sin estado |
| `Partida` | puntaje, duración, bajas, victoria/derrota |

### Sistemas

| Archivo | Qué hace |
|---|---|
| `GestorOleadas` | cuándo, dónde y de qué tipo aparecen los enemigos |
| `InventarioArmas` | dos slots, bonos por arma, reglas de reemplazo |
| `EstacionMejoras` | sortea ofertas por rareza y administra el menú |
| `EncuentroAleatorio` | cofres y estaciones por zonas de 1200 px |
| `Escenario` | fondo y cámara que sigue al jugador |

### Persistencia

| Archivo | Guarda |
|---|---|
| `ArchivoJugadores` | perfil: nivel máximo, puntaje máximo, chatarra |
| `ArchivoMejoras` | catálogo de mejoras y su nivel actual |
| `ArchivoEnemigos` | catálogo de variantes de enemigo |
| `ArchivoPartidas` | historial de partidas jugadas |

### Presentación

`Juego` coordina todo y dibuja. Los `Aspecto*` (`AspectoJugador`,
`AspectoArmas`, `AspectoProyectiles`, `AspectoEncuentros`) cargan las texturas
y resuelven la parte visual de cada cosa.

**Cuidado:** `Juego.cpp` tiene ~1500 líneas y concentra entrada, menús, cobros,
colisiones, recompensas, HUD y dibujo. Es el archivo más difícil de explicar, y
el más probable de tocar en cualquier modificación.

---

## 4. Los tipos compartidos

```cpp
struct Posicion      { float x, y; };
struct FormaColision { float x, y, ancho, alto; };
struct Recompensa    { int experiencia, chatarra; };
```

**Por qué existen:** son el vocabulario con el que los tres módulos se hablan
sin tocarse los atributos privados. `Enemigo` no conoce a `Jugador`; recibe un
`Posicion` y devuelve un `FormaColision`.

**Por qué en un archivo aparte:** si cada uno definiera su propia `Posicion`, al
integrar habría dos definiciones distintas del mismo tipo y no compilaría.

**Convención clave, acordada por escrito:** en `FormaColision`, `x` e `y` son la
**esquina superior izquierda**, no el centro. Si un módulo asumiera el centro,
todas las colisiones quedarían corridas media entidad.

El `Jugador` trabaja internamente con posición central, así que al calcular el
objetivo del enemigo se hace la conversión:

```cpp
objetivo.x = posicionJugador.x - forma.ancho / 2.0f;
```

---

## 5. Los módulos, uno por uno

### Jugador (Nicolás)

Movimiento con WASD o flechas, vida, experiencia, niveles y chatarra.

**Progresión:** al ganar experiencia se llama a `verificarSubidaNivel()`, que
usa un `while` (no un `if`) porque una recompensa grande puede hacer subir
varios niveles de golpe:

```cpp
while (_experiencia >= _experienciaSiguienteNivel) {
    _experiencia -= _experienciaSiguienteNivel;
    _nivel++;
    _experienciaSiguienteNivel += 50;
    _vidaMaxima += 20;
    _vida = _vidaMaxima;
    guardarProgreso();
}
```

**Al construirse lee el disco:** el nivel y la chatarra salen de
`Jugadores.dat`, y la vida máxima y la velocidad se reconstruyen sumando las
mejoras de `Mejoras.dat`. O sea que el progreso sobrevive entre partidas.

**Punto débil conocido:** `Jugador.h` incluye SFML y guarda su propia
`sf::RectangleShape`. Es la única entidad que mezcla lógica con presentación;
el resto delega el dibujo. Está anotado como pendiente de uniformar.

### Enemigo y GestorOleadas (Ramiro)

Ver `notas-modulo-enemigos.md` y `guia_oleadas_enemigos.md` para el detalle.
Lo esencial:

**Las cuatro variantes no son cuatro clases.** Son la misma clase `Enemigo` con
distintos valores, elegidos por un `enum`:

```cpp
enum TipoEnemigo { BASICO, RAPIDO, RESISTENTE, JEFE };
```

| Variante | id | Tamaño | Vida | Velocidad | Daño | Exp/Chatarra |
|---|---|---|---|---|---|---|
| Básico | 1 | 56×56 | 100 | 120 px/s | 10 | 5 / 3 |
| Rápido | 2 | 40×40 | 60 | 220 px/s | 8 | 7 / 2 |
| Resistente | 3 | 72×72 | 250 | 70 px/s | 15 | 12 / 6 |
| Jefe | 4 | 120×120 | 3000 | 85 px/s | 35 | 200 / 120 |

**La persecución** es el único cálculo con matemática del módulo:

```cpp
float dx = objetivo.x - x;                   // 1. hacia dónde
float dy = objetivo.y - y;
float distancia = std::sqrt(dx*dx + dy*dy);  // 2. Pitágoras

if (distancia > 0.0f) {
    float paso = velocidad * dt;
    if (paso >= distancia) {
        x = objetivo.x;                      // 3. llegó: se planta
        y = objetivo.y;
    } else {
        dx /= distancia;                     // 4. normalizar
        dy /= distancia;
        x += dx * paso;
        y += dy * paso;
    }
}
```

- **Normalizar** = dividir por la distancia para quedarse solo con la dirección.
  Sin eso, el enemigo iría más rápido cuanto más lejos está.
- **El `if (distancia > 0)`** evita dividir por cero, que en `float` da `NaN` —
  y `NaN` se contagia a toda cuenta que lo toque, arruinando la posición para
  siempre.
- **El `paso >= distancia`** evita que vibre al llegar: sin eso se pasa de largo
  y vuelve, 60 veces por segundo.

**La dificultad sube por tiempo, no por bajas.** Una sola fórmula gobierna todo:

```cpp
progreso = tiempoPartida / 300    // 0 al empezar, 1 al minuto 5
intervalo = 1.5 - (1.5 - 0.4) * progreso   // baja de 1.5s a 0.4s
tope      = 15  + (60  - 15 ) * progreso   // sube de 15 a 60 vivos
```

| min | oleada | intervalo | tope | básicos | rápidos | resist. |
|---|---|---|---|---|---|---|
| 0 | 1 | 1.50 | 15 | sí | — | — |
| 1 | 3 | 1.28 | 24 | sí | sí | — |
| 3 | 7 | 0.84 | 42 | sí | sí | sí |
| 5 | 11 | 0.40 | 60 | sí | sí | sí + **JEFE** |

**Las oleadas son tramos de 30 segundos**, no grupos de enemigos: no hay que
matarlos a todos para avanzar. Con oleadas cerradas, un enemigo trabado en una
esquina bloquearía la partida para siempre.

**Aparecen en un anillo** de 700 px alrededor del jugador, en un ángulo al azar:

```cpp
x = jugador.x + cos(angulo) * 700;
y = jugador.y + sin(angulo) * 700;
```

No se usan los bordes de la ventana porque la cámara sigue al jugador: los
bordes de la pantalla ya no son los bordes del mundo.

**El techo de enemigos vivos** es lo que evita que el juego se arrastre. El jefe
es la excepción: se genera **fuera** del techo, porque si estuviera lleno nunca
saldría y la partida no se podría ganar.

### Arma, Proyectil y colisiones (Lucas)

**Disparo automático:** `Juego` busca el enemigo vivo más cercano dentro del
alcance, y le pide al arma que dispare hacia ahí. El arma valida el alcance y
calcula la dirección normalizada.

**El enfriamiento** (cadencia) es un temporizador por acumulación:
`velocidadAtaque = 4.0f` significa 4 ataques por segundo, o un intervalo de
`1/4 = 0.25` segundos.

Las cuatro armas:

| Arma | id | Daño | Cadencia | Alcance | Especial |
|---|---|---|---|---|---|
| Pistola | 1 | 25 | 2.0/s | 600 | +1 proyectil (máx 5) |
| Metralleta | 2 | 6 | 8.0/s | 600 | +1 proyectil (máx 5) |
| Escopeta | 3 | 12 | 0.8/s | 300 | +1 perdigón (máx 9) |
| Cañón de energía | 4 | 60 | 0.5/s | 800 | +1 enemigo atravesado (máx 6) |

**`SistemaColisiones`** compara rectángulos (AABB) y no conoce ninguna clase de
entidad. Sus métodos son `static`: no hace falta crear un objeto para usarlo.

```cpp
return primera.x < segunda.x + segunda.ancho &&
       segunda.x < primera.x + primera.ancho &&
       primera.y < segunda.y + segunda.alto &&
       segunda.y < primera.y + primera.alto;
```

Con `<` estricto, dos bordes que apenas se tocan **no** cuentan como choque.

**El cañón atraviesa enemigos** y no debe pegarle dos veces al mismo. Para eso
cada enemigo tiene un identificador único de instancia y el proyectil recuerda a
quiénes golpeó:

```cpp
proyectil.registrarImpacto(enemigo.obtenerIdInstancia())
```

Ojo con no confundir: `obtenerIdEnemigo()` devuelve la **variante** (1-4),
`obtenerIdInstancia()` identifica a **ese enemigo puntual** (1, 2, 3…).

### Rarezas y estación de mejoras (Lucas)

La estación sortea tres ofertas; cada una tiene su propia rareza:

| Rareza | Peso | Daño | Cadencia | Especial |
|---|---|---|---|---|
| Común | 60% | +5% | +4% | no |
| Rara | 25% | +10% | +8% | 15% |
| Épica | 12% | +18% | +12% | 50% |
| Legendaria | 3% | +25% | +18% | garantizado |

**Los porcentajes se suman sobre la base, no se componen.** Dos comunes dan
+10% de daño, no +10.25%. La cuenta se hace en `long long` y satura en
`INT_MAX`, porque multiplicar dos `int` grandes desborda en silencio y daría un
número negativo.

Hay **dos slots** de arma. La pistola arranca en el slot 1 y es la única que se
puede reemplazar; una vez reemplazada, los dos slots quedan fijos y solo se
ofrecen mejoras de lo que ya se tiene.

---

## 6. Los cinco archivos binarios

Todos siguen la misma forma: un struct plano, `fopen` en modo binario,
`fwrite`/`fread` del struct completo, y valores por defecto si el archivo no
existe.

| Archivo | Tipo | Modo | Tamaño del registro |
|---|---|---|---|
| `Jugadores.dat` | perfil (uno) | `"wb"` / `"rb"` | 12 bytes |
| `Mejoras.dat` | catálogo | `"wb"` / `"rb"` | 144 bytes |
| `Enemigos.dat` | catálogo | `"wb"` / `"rb"` | 64 bytes |
| `Partidas.dat` | **historial** | `"ab"` para agregar | 32 bytes |
| `Armas.dat` | catálogo | *pendiente* | — |

### Por qué structs planos

Solo `int`, `float`, `bool` y arreglos de `char`. **Nunca `std::string`:** un
string guarda internamente un *puntero* a otra parte de la memoria. Si se vuelca
crudo al archivo, se guarda una dirección que en la próxima ejecución no
significa nada.

### Catálogo vs historial

```cpp
fopen(ruta, "wb");   // catálogo: reescribe todo
fopen(ruta, "ab");   // historial: agrega al final
```

`Partidas.dat` usa `"ab"` porque cada partida suma una fila. Con `"wb"` cada
partida pisaría el historial entero.

### Leer sin cargar basura

```cpp
while (fread(&registro, sizeof(RegistroPartida), 1, archivo) == 1) {
```

`fread` devuelve cuántos registros **completos** leyó. Si el archivo quedó
cortado por la mitad, el pedazo incompleto se descarta en lugar de entrar como
basura.

### Dónde está la carpeta `datos/`

**`fopen` no crea carpetas.** Si no existe `datos/`, devuelve `NULL` y el
guardado se pierde sin ningún aviso.

Y la ruta `"datos/..."` se resuelve contra el **directorio de trabajo**, que no
es necesariamente donde está el `.exe`. Al ejecutar con doble clic sobre
`bin\Debug\ProyectoTPI.exe`, el directorio es `bin\Debug`.

`CarpetaDatos.h` lo resuelve buscando el archivo de proyecto:

```cpp
if (_access("ProyectoTPI.cbp", 0) == 0)       return "datos";
if (_access("../../ProyectoTPI.cbp", 0) == 0) return "../../datos";
```

Así la raíz y `bin/Debug` comparten los mismos archivos. Además
`asegurarCarpetaDatos()` llama a `_mkdir` antes de cada escritura, y la variable
de entorno `TPI_CARPETA_DATOS` permite correr pruebas sin tocar el progreso
real.

### Qué se versiona en git

- **Sí:** `Enemigos.dat` — es contenido del juego.
- **No:** `Jugadores.dat`, `Mejoras.dat`, `Partidas.dat` — son el progreso de
  cada uno; versionarlos daría conflictos binarios en cada partida.

---

## 7. Conceptos de C++ para defender

| Pregunta probable | Respuesta corta |
|---|---|
| ¿Por qué `.h` y `.cpp` separados? | El `.h` **declara** qué sabe hacer la clase (el contrato); el `.cpp` **define** cómo lo hace. Quien usa la clase solo incluye el `.h`. |
| ¿Qué hace `#pragma once`? | Evita que el header se incluya dos veces en la misma compilación, lo que daría error de redefinición. Equivale a los `#ifndef / #define / #endif`. |
| ¿Qué es `Enemigo::` en el `.cpp`? | Dice que esa función es un **método de la clase**, no una función suelta. Por eso adentro puede usar `x`, `vida`, etc. |
| ¿Qué significa `const` al final de un método? | Promete que el método **no modifica el objeto**. Si por error se escribe `x = 0;` adentro, no compila. Tiene que estar en el `.h` **y** en el `.cpp`. |
| `private` vs `public` | `private`: solo la clase lo toca. `public`: lo usa cualquiera. Es el **encapsulamiento**: la vida solo baja por `recibirDanio()`, nadie puede escribir `enemigo.vida = 9999`. |
| ¿Qué es un método `static`? | Pertenece a la clase, no a un objeto: se llama sin instancia (`SistemaColisiones::hayInterseccion(...)`). |
| ¿Qué es un **atributo** `static`? | Una sola variable compartida por todos los objetos de la clase. Se usa para el contador de `idInstancia`: cada enemigo nuevo se lleva el número y lo incrementa. |
| ¿Por qué `char nombre[30]` y no `std::string`? | Para poder guardarlo en un archivo binario: un `string` guarda un puntero, no el texto. |
| ¿Qué es el `'\0'`? | El cero que marca el fin de una cadena en C. Sin él, quien la lea sigue de largo por la memoria. `strncpy` no lo agrega si el texto llega al límite, por eso se fuerza a mano. |
| ¿Por qué `std::vector` y no un arreglo fijo? | La cantidad de enemigos y proyectiles varía durante la partida. El vector crece y libera su memoria solo (RAII). |
| ¿Por qué el `&` en `for (Enemigo& e : enemigos)`? | Sin el `&` se trabaja sobre una **copia**: se mueve la copia y el original queda quieto. No da error ni warning. |
| ¿Por qué no se puede borrar del vector mientras se recorre? | Al borrar, los elementos de atrás se corren un lugar y el recorrido queda apuntando a otra cosa: se saltean elementos o el programa se rompe. Hay que hacerlo en una pasada aparte. |
| ¿Qué hace `erase` + `remove_if`? | `remove_if` **no borra**: empuja los que cumplen la condición al final y devuelve dónde arranca esa zona; el `erase` corta desde ahí. |
| ¿Qué es una lambda? | Una función anónima escrita en el lugar donde se usa: `[](const Enemigo& e) { return !e.estaVivo(); }`. |
| ¿Por qué pasar `const Tipo&` en vez de `Tipo`? | Para no copiar el objeto en cada llamada, y el `const` garantiza que la función no lo modifica. |
| ¿Qué es un `enum`? | Una lista cerrada de valores con nombre. Si se escribe mal, no compila — con un `string` compilaría y fallaría en ejecución. |
| ¿Por qué el `default` en un `switch` que devuelve algo? | Toda función con retorno tiene que devolver algo por **todos** los caminos. Sin `default`, el compilador avisa que podría no retornar nada. |
| ¿Por qué `sizeof(RegistroEnemigo)` es 64 y no 36? | Por el **padding**: el compilador alinea los campos en memoria, así que el struct ocupa más que la suma de sus partes. Importa porque el tamaño del registro define el formato del archivo. |

---

# PARTE 2 — Ejercicios de modificación

Así se usan: leer la consigna, **intentar resolverla sin mirar la solución**, y
después comparar. Lo que se evalúa no es recordar la respuesta sino saber
*dónde buscar*.

Al final de cada ejercicio hay un **"cuidado con"**: lo que se rompe si se toca
solo la mitad.

---

## Nivel 1 — Cambiar un valor de balance

Son los más probables y los más simples: casi todo el balance está en
constantes con nombre.

### E1. Que el jefe aparezca a los 3 minutos en vez de 5

<details>
<summary>Solución</summary>

`src/GestorOleadas.cpp`, arriba del todo:

```cpp
const float GestorOleadas::SEGUNDOS_APARECE_JEFE = 300.0f;  // → 180.0f
```

**Cuidado con:** la curva de dificultad sigue llegando a su máximo a los 300 s
(`SEGUNDOS_DIFICULTAD_MAXIMA`). Hoy los dos valores coinciden a propósito: el
jefe llega cuando la dificultad terminó de subir. Si se adelanta solo el jefe,
la pelea final pasa a ser más fácil que los minutos que vienen después.
Para mantener la coherencia hay que mover las dos constantes.

**Cómo verificar:** `PruebaEnemigos.exe` y apretar **T** (adelanta 30 s) seis
veces en vez de diez.
</details>

### E2. Que los enemigos aparezcan al doble de velocidad al final de la partida

<details>
<summary>Solución</summary>

`src/GestorOleadas.cpp`:

```cpp
const float GestorOleadas::INTERVALO_APARICION_MINIMO = 0.4f;  // → 0.2f
```

**Cuidado con:** el techo de vivos (`MAXIMO_VIVOS_FINAL = 60`) no cambia, así
que a partir de cierto punto el temporizador corre al vacío porque las
apariciones se descartan por el tope. Si se quiere el doble de enemigos en
pantalla, hay que subir también el techo — y ahí conviene probar que el juego
no empiece a andar lento.
</details>

### E3. Que el resistente aparezca desde el primer minuto

<details>
<summary>Solución</summary>

`src/GestorOleadas.cpp`:

```cpp
const float GestorOleadas::SEGUNDOS_APARECE_RESISTENTE = 180.0f;  // → 60.0f
```

**Cuidado con:** quedaría igual que `SEGUNDOS_APARECE_RAPIDO`, así que en el
segundo 60 se habilitan las dos variantes a la vez. Revisar
`variantePorAparecer()`: la lógica de `habilitadas` funciona igual, pero el
salto de dificultad se siente de golpe.
</details>

### E4. Que la victoria valga el doble de puntos

<details>
<summary>Solución</summary>

`src/Partida.cpp`:

```cpp
const int Partida::PUNTOS_POR_VICTORIA = 5000;  // → 10000
```

**Cuidado con:** nada se rompe. El puntaje se **recalcula entero** cada vez que
cambia un contador, así que no hay valores viejos arrastrados. Las partidas ya
guardadas en `Partidas.dat` conservan su puntaje original: el archivo guarda el
resultado, no la fórmula.
</details>

### E5. Que el jugador empiece con más vida

<details>
<summary>Solución</summary>

`src/Jugador.cpp`, en el constructor:

```cpp
_vidaMaxima = 100 + (_nivel - 1) * 20;   // el 100 es la base
```

**Cuidado con:** el mismo valor aparece en `restablecerProgreso()`, que llama a
`cargarEstado(100, 1, 0, 0)`. Si se cambia uno solo, "Nueva Partida" deja al
jugador con la vida vieja. Es un número repetido en dos lugares: buen momento
para proponer una constante.
</details>

---

## Nivel 2 — Agregar un elemento a algo que ya existe

### E6. Agregar un quinto tipo de enemigo (por ejemplo, "Robot explosivo")

<details>
<summary>Solución</summary>

Hay que tocar **cinco lugares**:

1. **`include/Enemigo.h`** — sumar al enum, antes del `JEFE` para no cambiarle
   el id:
   ```cpp
   enum TipoEnemigo { BASICO, RAPIDO, RESISTENTE, EXPLOSIVO, JEFE };
   ```
2. **`src/Enemigo.cpp`** — un `case` nuevo en `crearEnemigo()` con sus stats.
3. **`src/ArchivoEnemigos.cpp`** — una fila más en `catalogoDeFabrica()`, y
   ampliar el rango de `registroEsUsable()`:
   ```cpp
   registro.idEnemigo >= 1 && registro.idEnemigo <= 5
   ```
4. **`src/GestorOleadas.cpp`** — habilitarlo en `variantePorAparecer()` (con su
   propia constante `SEGUNDOS_APARECE_EXPLOSIVO`).
5. **`src/Juego.cpp`** — un color en `obtenerColorEnemigo()`, si no se dibuja
   con el color por defecto.

**Cuidado con tres cosas:**

- **Insertarlo antes del `JEFE` rompe la correspondencia con el catálogo.**
  `registroDe()` asume `id = (int)tipo + 1`. Si `JEFE` pasa de 3 a 4 en el enum,
  `registroDe(JEFE)` buscaría la fila con id 5, que no existe. No hay error ni
  crash: `fabricar()` cae en el `crearEnemigo()` del código y el jefe **deja de
  leer sus valores del archivo**, en silencio. Lo seguro es agregar la variante
  nueva **al final** del enum, o resolver el ejercicio E16.
- **`Enemigos.dat` ya existe en disco con 4 filas.** El catálogo de fábrica con
  5 solo se escribe si el archivo falta. Hay que borrar `datos/Enemigos.dat`
  para que se regenere, o el enemigo nuevo nunca va a aparecer.
- **`detectarJefeDerrotado()` compara contra el id 4** vía `idInstanciaJefe`, y
  `Juego::actualizarPartida` compara `obtenerIdEnemigo() == 4` para el jefe. Si
  el jefe cambia de id, esos dos lugares quedan desactualizados.
</details>

### E7. Agregar una mejora nueva (por ejemplo, "Regeneración")

<details>
<summary>Solución</summary>

1. **`src/Jugador.cpp`**, en `restablecerProgreso()`, sumar al catálogo inicial:
   ```cpp
   {3, "Regeneracion", 0, 3, "Recuperas vida con el tiempo"}
   ```
2. **`src/ArchivoMejoras.cpp`**, el mismo agregado en el catálogo por defecto
   (el que se escribe si el archivo no existe).
3. **Constructor de `Jugador`** — aplicar su efecto al cargar:
   ```cpp
   else if (mejora.idMejora == 3) { /* guardar la tasa de regeneración */ }
   ```
4. **`Jugador::actualizar()`** — si la mejora da vida con el tiempo, acá va el
   temporizador por acumulación.

**Cuidado con:** `Mejoras.dat` ya existe con 2 filas. Como el archivo se lee
entero con un `while (fread(...))`, agregar una fila al catálogo de fábrica no
afecta a quien ya tiene el archivo viejo: hay que borrarlo o usar la tecla de
"Nueva Partida" (**N**), que llama a `restablecerProgreso()` y lo reescribe.
</details>

### E8. Agregar una quinta arma

<details>
<summary>Solución</summary>

Es el ejercicio con más puntos de contacto. Mínimo:

1. **`include/Juego.h`** — sumar al `enum class TipoArmaPrueba` y declarar el
   `Arma` nueva como atributo.
2. **`src/Juego.cpp`** — construirla en la lista de inicialización, y agregarla
   a `obtenerArmaPorId()` y `seleccionarArmaPrueba()`.
3. **`src/InventarioArmas.cpp`** — acá está la trampa: el rango
   `idArma >= 1 && idArma <= 4` está repetido en **cinco** métodos
   (`poseeArma`, `puedeRecibirArma`, `obtenerBonificaciones`,
   `OfertaArma::esValida`, `obtenerNombreArma`). Hay que ampliarlo en todos.
   Además los arreglos `std::array<..., 4>` pasan a 5.
4. **`AspectoArmas`** — su textura, si tiene gráfico propio.

**Cuidado con:** los límites de especiales están escritos en **tres** lugares
distintos (`InventarioArmas::puedeMejorarEspecial`,
`Arma::configurarMejoras` y `Arma::obtenerCantidadProyectiles`), cada uno con su
propio `_id == 4 ? 3 : 4`. Si se toca uno solo quedan desincronizados sin que
nada avise.
</details>

### E9. Mostrar la oleada y el tiempo en el HUD con formato mm:ss

<details>
<summary>Solución</summary>

`src/Juego.cpp`, en `dibujarInterfaz()`. Hoy muestra los segundos crudos:

```cpp
"OLEADA: " + std::to_string(_gestorOleadas.oleadaActual()) +
" | TIEMPO: " + std::to_string((int)_gestorOleadas.tiempoDePartida())
```

Para mm:ss hay que dividir por 60 y rellenar con un cero:

```cpp
const int s = (int)_gestorOleadas.tiempoDePartida();
std::string reloj = std::to_string(s / 60) + ":" +
                    (s % 60 < 10 ? "0" : "") + std::to_string(s % 60);
```

**Cuidado con:** no hace falta tocar `GestorOleadas`. El gestor **informa**
segundos; el formato es decisión de quien dibuja. Confundir esas dos
responsabilidades es el error de diseño típico.
</details>

---

## Nivel 3 — Agregar una característica nueva

### E10. Que los enemigos no se superpongan entre sí

<details>
<summary>Solución</summary>

Hoy todos persiguen el mismo punto, así que se apilan unos sobre otros.

La solución mínima va en `GestorOleadas::moverEnemigos()`: después de mover a
todos, recorrer los pares y separar los que se intersecan, usando el sistema que
ya existe:

```cpp
if (SistemaColisiones::hayInterseccion(a.obtenerFormaColision(),
                                       b.obtenerFormaColision())) {
    // empujar a cada uno en direcciones opuestas
}
```

**Cuidado con tres cosas:**

- `GestorOleadas` no incluye `SistemaColisiones.h` todavía: hay que agregarlo al
  header (y es una dependencia nueva entre módulos, hay que avisar al equipo).
- Comparar todos contra todos es **O(n²)**: con 60 enemigos son 1770
  comparaciones por frame. Funciona, pero conviene medirlo.
- `Enemigo` no tiene un método para empujarlo: su posición solo cambia por
  `actualizar()`. Habría que agregar algo como `desplazar(float dx, float dy)`,
  y eso **cambia el contrato de la clase** — hay que justificar por qué no
  rompe el encapsulamiento.
</details>

### E11. Que el jefe tenga una barra de vida en pantalla

<details>
<summary>Solución</summary>

Falta un dato: **`Enemigo` no expone su vida.** Tiene `estaVivo()` pero no
`obtenerVida()`, y para una barra hace falta la vida actual **y** la máxima.

1. **`include/Enemigo.h`** — agregar `vidaMaxima` como atributo y dos getters:
   ```cpp
   int obtenerVida() const;
   int obtenerVidaMaxima() const;
   ```
2. **`src/Enemigo.cpp`** — guardar `vidaMaxima = _vida` en el constructor.
3. **`src/Juego.cpp`** — dibujar dos rectángulos (fondo y relleno) cuando
   `_gestorOleadas.jefeEstaVivo()`, con el ancho proporcional a
   `vida / vidaMaxima`.

**Cuidado con:** la división entre dos `int` da `int` (0 o 1). Hay que convertir
a `float` antes: `(float)vida / (float)vidaMaxima`.

**Y con:** `vidaMaxima` no está en `Enemigos.dat`. No hace falta agregarlo —
la vida del catálogo *es* la máxima al nacer.
</details>

### E12. Que se puedan ver las últimas 5 partidas en el menú

<details>
<summary>Solución</summary>

La parte difícil ya está hecha: `ArchivoPartidas::leerTodas()` devuelve el
historial completo.

`src/Juego.cpp`, en `dibujarMenuPrincipal()`:

```cpp
ArchivoPartidas historial;
const std::vector<RegistroPartida> partidas = historial.leerTodas();

// recorrer las últimas 5, desde el final
const std::size_t desde = partidas.size() > 5 ? partidas.size() - 5 : 0;
for (std::size_t i = desde; i < partidas.size(); i++) {
    // dibujar puntaje, duración y si fue victoria
}
```

**Cuidado con:** no leer el archivo **dentro** del bucle de dibujo. Se abriría
el archivo 60 veces por segundo. Conviene leerlo una vez al entrar al menú y
guardar el resultado.
</details>

### E13. Que el jugador tenga un dash (esquive rápido)

<details>
<summary>Solución</summary>

Toca el módulo de Nicolás.

1. **`include/Jugador.h`** — atributos nuevos: `_tiempoDash` (cuánto dura el
   impulso) y `_enfriamientoDash`.
2. **`src/Jugador.cpp`** — en `procesarEntrada()`, detectar la tecla; mientras
   el dash esté activo, multiplicar la velocidad.
3. Los dos temporizadores bajan con el `dt`.

**Cuidado con:** `Jugador::actualizar()` **no recibe `dt`** — su firma es
`actualizar(bool limitarALaVentana)` y el movimiento usa la velocidad sin
multiplicar por el tiempo. Para los temporizadores del dash hace falta el
delta time, así que habría que cambiar la firma a `actualizar(float dt, bool
limitarALaVentana)` y ajustar las llamadas en `Juego.cpp` y en
`pruebas/Jugador.cpp`.

Eso es **cambiar un contrato público**: hay que avisar al equipo antes.
</details>

---

## Nivel 4 — Cambiar un contrato o un formato de archivo

### E14. Guardar en `Partidas.dat` cuántos disparos se hicieron

<details>
<summary>Solución</summary>

Es el ejercicio más delicado del conjunto, porque **cambia el formato del
archivo**.

1. **`include/ArchivoPartidas.h`** — un campo más en `RegistroPartida`.
2. **`include/Partida.h` / `src/Partida.cpp`** — un contador, su
   `registrarDisparo()` y su getter.
3. **`src/ArchivoPartidas.cpp`** — copiar el campo nuevo en `agregar()`, y
   validarlo en `registroValido()`.
4. **`src/Juego.cpp`** — llamar a `registrarDisparo()` donde se dispara.

**El cuidado principal:** agregar un campo cambia `sizeof(RegistroPartida)` (de
32 a 36 bytes, por el padding). El archivo viejo tiene registros de 32 bytes, y
`fread` los va a leer **desalineados**: cada lectura tomaría pedazos de dos
registros distintos y devolvería números sin sentido.

Hay tres salidas, y conviene saber nombrarlas:

- **Borrar el archivo** (aceptable acá: es historial de pruebas).
- **Versionar el formato**: guardar un número de versión al principio del
  archivo y leer distinto según cuál sea.
- **Migrar**: leer con el formato viejo y reescribir con el nuevo.

Para el TPI, lo correcto es *decir* que el archivo viejo queda inválido y por
qué. Esa es la respuesta que se espera.
</details>

### E15. Que la victoria sea sobrevivir 10 minutos en vez de matar al jefe

<details>
<summary>Solución</summary>

La condición vive en `Juego::actualizarPartida()`:

```cpp
if (!jugador.estaVivo())      _partida.marcarDerrota();
else if (jefeMuerto)          _partida.marcarVictoria();
```

Se reemplaza la segunda por `_gestorOleadas.tiempoDePartida() >= 600.0f`.

**Cuidado con:** `Partida` ya está preparada para esto — **el primer desenlace
que llega define el resultado** y no se puede pisar:

```cpp
void Partida::marcarVictoria() {
    if (resultado == ResultadoPartida::EnCurso) { ... }
}
```

Sin esa guarda, morir en el mismo frame en que se cumple la condición daría un
resultado distinto según el orden en que se revisen los `if`. Vale explicarlo:
es una decisión de diseño, no una casualidad.

**Y con:** si la victoria ya no depende del jefe, hay que decidir qué hacer con
él — ¿sigue apareciendo como enemigo fuerte, o se saca? Si se saca, sobra
`generarJefeSiCorresponde()`.
</details>

### E16. Que `Enemigo` no dependa del orden del `enum`

<details>
<summary>Solución</summary>

Hoy `GestorOleadas::registroDe()` asume que el id del catálogo es el valor del
enum más uno:

```cpp
const int idBuscado = (int)tipo + 1;
```

Funciona, pero acopla el orden del `enum` al contenido del archivo: reordenar el
enum (o insertar una variante en el medio) rompe la correspondencia en silencio.

La forma robusta es una función explícita:

```cpp
int idDeVariante(TipoEnemigo tipo) {
    switch (tipo) {
        case BASICO:     return 1;
        case RAPIDO:     return 2;
        case RESISTENTE: return 3;
        case JEFE:       return 4;
    }
    return 1;
}
```

**Por qué es mejor:** si mañana se agrega una variante al medio del enum, el
`switch` sigue dando los ids correctos. Y si se agrega al enum y se olvida el
`case`, el compilador avisa (con `-Wall`, un `switch` sobre un enum sin todos
los casos da warning).

Es un buen ejercicio para la pregunta *"¿qué mejorarías de tu código?"*.
</details>

---

## Mapa rápido: "si el profe pide X, voy a…"

| Pide | Archivo |
|---|---|
| cambiar vida/velocidad/daño de un enemigo | `ArchivoEnemigos.cpp` (`catalogoDeFabrica`) + borrar `datos/Enemigos.dat` |
| cambiar cuándo aparece algo | `GestorOleadas.cpp`, constantes de arriba |
| cambiar el puntaje | `Partida.cpp`, las tres constantes |
| cambiar stats de un arma | `Juego.cpp`, la lista de inicialización del constructor |
| cambiar rarezas o porcentajes | `InventarioArmas.cpp` (`obtenerPorcentajeDano`, `obtenerPorcentajeCadencia`) |
| cambiar precios de cofres/estaciones | `Juego.cpp` (`_costoCofre`) y `EstacionMejoras.cpp` |
| cambiar la vida o velocidad del jugador | `Jugador.cpp`, constructor |
| tocar el HUD o los menús | `Juego.cpp` (`dibujarInterfaz`, `dibujarMenuPrincipal`, `dibujarResultado`) |
| tocar colisiones | `SistemaColisiones.cpp` |
| agregar/quitar un archivo del proyecto | `ProyectoTPI.cbp` **y** `compilar_debug.bat` **y** `compilar_pruebas.bat` |

---

## Trampas conocidas del proyecto

Vale tenerlas a mano: si el profe pide algo que las toca, conviene nombrarlas
antes de que las encuentre él.

1. **`Enemigos.dat` no se regenera solo.** El catálogo de fábrica se escribe
   únicamente si el archivo falta. Cambiar los valores en el código sin borrar
   el `.dat` no tiene efecto.
2. **El rango `idArma 1..4`** está repetido en cinco métodos de
   `InventarioArmas`.
3. **Los límites de especiales** están en tres lugares distintos.
4. **`Jugador` incluye SFML** y guarda su propia figura; el resto de las
   entidades delega el dibujo.
5. **`Jugador::actualizar()` no recibe `dt`**: su movimiento no depende del
   tiempo, a diferencia de todo el resto.
6. **`Juego.cpp` tiene ~1500 líneas** y concentra demasiadas
   responsabilidades. Es la respuesta honesta a *"¿qué refactorizarían?"*.
7. **`Armas.dat` todavía no existe**: es el quinto binario y está pendiente.
8. **La semilla del azar es fija** en `GestorOleadas`, así que todas las
   partidas generan la misma secuencia de ángulos y variantes. Para que cada
   partida sea distinta: `std::random_device{}()`.

---

## Cómo probar cada módulo por separado

Cada módulo tiene su ejecutable aislado, con su `.bat` en `pruebas/`. Sirve para
demostrar una parte sin levantar el juego entero.

| `.bat` | Qué demuestra |
|---|---|
| `compilar_pruebas.bat` | enemigos, oleadas, jefe y partida (raíz del proyecto) |
| `probar_jugador.bat` | movimiento, vida, niveles |
| `probar_colisiones.bat` | intersecciones, sin ventana |
| `probar_proyectil.bat` | movimiento y alcance de un disparo |
| `probar_arma.bat` | cadencia y dirección |
| `probar_armas_nuevas.bat` | escopeta y cañón |
| `probar_inventario_armas.bat` | slots, rarezas y reemplazo |
| `probar_estacion_mejoras.bat` | menú de ofertas |
| `probar_encuentros.bat` | cofres y estaciones por zonas |
| `probar_camara.bat` | seguimiento del jugador |
| `probar_menu_principal.bat` | menú, pausa y controles |
| `probar_persistencia.bat` | los archivos binarios |
| `probar_recompensas.bat` | experiencia y chatarra al morir |
| `probar_enemigo_proyectil.bat` | impacto proyectil-enemigo |
| `probar_juego_proyectiles.bat` | disparos simultáneos |
| `probar_aspecto_jugador.bat` | animación del robot |
| `probar_aspecto_armas.bat` | gráficos de las armas |
| `probar_aspecto_proyectiles.bat` | texturas de los disparos |
| `probar_aspecto_encuentros.bat` | gráficos de cofres y estaciones |
| `probar_partida_integrada.bat` | todo junto |

En `PruebaEnemigos.exe`: **T** adelanta 30 segundos (para llegar al jefe sin
esperar 5 minutos), **Espacio** daña a todos, **K** daña al jugador simulado,
**R** reinicia.

---

## Controles del juego

| Tecla | Acción |
|---|---|
| WASD / flechas | mover |
| 1 / 2 | seleccionar slot de arma |
| F | interactuar con cofre o estación (a menos de 100 px) |
| 1 / 2 / 3 | en la estación: elegir oferta |
| Escape | cancelar compra, o pausar |
| Enter / flechas | navegar los menús |
| **R** | reiniciar la prueba |
| **K** | daño al jugador (prueba) |
| **E** | +40 de experiencia (prueba) |
| **U** | comprar un nivel de vida máxima (prueba) |
| **N** | nueva partida: formatea el progreso (prueba) |

Las últimas cinco son teclas de prueba, no del juego final.
