# Notas del módulo Enemigos

Apuntes de lo que vamos armando y por qué. Escrito para entenderlo de nuevo
dentro de dos meses, y para poder explicarlo en la defensa.

---

## 1. Qué es un juego, en realidad

Un juego es un bucle que se repite muy rápido. En `src/Juego.cpp`:

```cpp
while (ventana.isOpen()) {
    procesarEventos();   // que apreto el usuario
    actualizar();        // mover todo un poquito
    dibujar();           // mostrar como quedo
}
```

Cada vuelta se llama **frame**, y hay 60 por segundo.

> **Ejemplo bobo:** es un librito de esos que dibujás en las esquinas de las
> hojas y al pasarlas rápido parece que el muñequito camina. Cada hoja es un
> frame. El juego dibuja 60 hojitas por segundo.

Todo el módulo de enemigos vive adentro del paso `actualizar()`.

---

## 2. El `dt` (delta time)

Es cuántos **segundos** pasaron desde el frame anterior. A 60 FPS vale ~0.016.

Sirve para medir la velocidad en píxeles **por segundo** en vez de por frame:

```cpp
x = x + 2;                    // mal: 2 pixeles por frame
x = x + 120 * dt;             // bien: 120 pixeles por segundo
```

Con la primera forma, en una compu lenta (30 FPS) el enemigo va a la mitad de
velocidad. Con la segunda, va igual en todas.

| FPS  | dt    | avanza por frame | en 1 segundo |
|------|-------|------------------|--------------|
| 60   | 0.016 | 1.92 px          | ~120 px      |
| 30   | 0.033 | 4 px             | ~120 px      |

> **Ejemplo bobo:** si caminás a 5 km/h, en media hora hacés 2,5 km — seas vos
> o sea tu abuela contando los pasos más despacio. El `dt` es esa "media hora":
> el tiempo real que pasó, no la cantidad de pasos.

Sale del game loop con `sf::Clock`:

```cpp
sf::Clock reloj;
float dt = reloj.restart().asSeconds();   // tiempo desde la ultima llamada
```

El `Enemigo` **no sabe nada de relojes ni de SFML**: solo recibe el número.

---

## 3. `TiposCompartidos.h` — el vocabulario entre módulos

> Nota: al principio este archivo se llamaba `Comunes.h` y lo había hecho yo.
> El equipo decidió quedarse con el `TiposCompartidos.h` de Lucas, que es
> idéntico, porque ya tenía dos clases incluyéndolo. El contenido es el mismo.

Tres structs que usan los tres módulos del grupo para hablar entre sí:

| Struct          | Qué lleva                 | Para qué |
|-----------------|---------------------------|----------|
| `Posicion`      | `x`, `y`                  | dónde está algo |
| `FormaColision` | `x`, `y`, `ancho`, `alto` | el rectángulo para detectar choques |
| `Recompensa`    | `experiencia`, `chatarra` | lo que da un enemigo al morir |

**Por qué en un archivo aparte:** si cada uno definiera su propia `Posicion`,
al integrar habría dos definiciones distintas y no compilaría. Un solo archivo
que todos incluyen resuelve el problema.

No se contienen ni se heredan entre sí: son tres cosas sueltas que comparten
archivo porque comparten propósito.

---

## 4. La clase `Enemigo`

### Atributos

Los siete primeros salen tal cual de `Enemigos.dat` (definido en el informe):

```cpp
int idEnemigo;              // que fila del catalogo es
char nombre[30];            // "Robot basico", "Robot rapido"...
int vida;                   // baja con el dano recibido
int dano;                   // cuanto lastima al tocar al jugador
float velocidad;            // pixeles por segundo
int experienciaOtorgada;    // lo que da al morir
int chatarraOtorgada;       // idem
float x, y;                 // donde esta AHORA (no va al archivo)
```

`x` e `y` no están en el `.dat` a propósito: el archivo guarda el **catálogo**
(qué tipos de enemigo existen), no dónde está parado cada uno en la partida.
Eso es estado temporal y vive solo en memoria.

> **Ejemplo bobo:** el catálogo es el menú del kiosco ("alfajor: $500").
> La posición es el alfajor puntual que está sobre el mostrador. El menú no
> dice dónde está cada alfajor.

### Por qué `char nombre[30]` y no `std::string`

Porque después hay que guardarlo en un archivo binario. Un `std::string`
guarda internamente un **puntero** a otra parte de la memoria; si lo volcás
crudo al `.dat`, guardás una dirección que mañana no significa nada. Los
arreglos de `char` de tamaño fijo se guardan tal cual y se recuperan bien.

---

## 5. Los métodos, uno por uno

### El constructor

```cpp
Enemigo::Enemigo(int _idEnemigo, float _x, ...) { ... }
```

Se ejecuta **solo**, cuando alguien escribe `Enemigo e(1, 100, 200, ...)`.
Se llama igual que la clase y no devuelve nada.

Sirve para que **no exista ningún enemigo sin sus datos cargados**. Sin
constructor, un enemigo recién creado tendría números basura de memoria
(tipo 3.700.000 de vida) y no entenderías por qué.

**El guion bajo en `_vida`:** el parámetro y el atributo conviven en la misma
función. Si los dos se llamaran igual, `vida = vida;` se asigna el parámetro a
sí mismo y el atributo nunca se carga. Compila sin avisar nada. El `_` los
distingue.

**La línea rara:**

```cpp
strncpy(nombre, _nombre, 30);
nombre[29] = '\0';
```

Un arreglo no se puede asignar con `=`, hay que copiarlo carácter por carácter.
Y `strncpy` no agrega el `'\0'` final si el texto llega justo al límite, por eso
se fuerza a mano.

**Qué es el `'\0'`:** en C, un texto termina cuando aparece un cero. Sin él,
quien lea la cadena sigue de largo por la memoria hasta encontrar un cero de
casualidad → basura en pantalla, o crash.

> **Ejemplo bobo:** es el punto final de una oración. Sin punto, el que lee
> sigue leyendo la página siguiente, el índice y la contratapa.

---

### `actualizar(Posicion objetivo, float dt)`

**El único método con matemática.** Se llama 60 veces por segundo y mueve al
enemigo un pasito hacia el objetivo.

En criollo: *"¿para qué lado está el objetivo? Dame un pasito para allá."*

```cpp
float dx = objetivo.x - x;                        // 1
float dy = objetivo.y - y;

float distancia = std::sqrt(dx*dx + dy*dy);       // 2

if (distancia > 0.0f) {                           // 3
    dx /= distancia;
    dy /= distancia;

    x += dx * velocidad * dt;                     // 4
    y += dy * velocidad * dt;
}
```

**1. La resta** da hacia dónde ir. El signo solo ya indica el sentido: si el
enemigo está en `x=100` y el objetivo en `x=300`, da `+200` (derecha); si el
objetivo estuviera en `x=50`, da `-50` (izquierda). Por eso no hace falta ningún
`if` para decidir para qué lado ir.

**2. Pitágoras** da la distancia en línea recta. `dx` y `dy` son los catetos, la
distancia es la hipotenusa.

**3. Normalizar** = dividir por la distancia para que el vector quede de largo 1.
Queda apuntando al mismo lado, pero sin el "qué tan lejos".

Pasa de *"andá 200 a la derecha y 300 abajo"* a *"andá 0.55 a la derecha y 0.83
abajo"*.

> **Ejemplo bobo:** alguien te pregunta dónde queda el baño. Le señalás con el
> dedo. El dedo apunta igual esté el baño a 3 metros o a 30 — la **dirección** no
> cambia con la distancia. Normalizar es quedarse solo con el dedo.

**Si te salteás este paso**, el enemigo se mueve más rápido cuanto más lejos está
y frena al acercarse. Se ve rarísimo y cuesta un montón darse cuenta de por qué.

**El `if`** evita dividir por cero cuando el enemigo llegó justo encima del
objetivo. En `float` eso no explota: devuelve `NaN` ("not a number"), que se
**contagia** a toda cuenta que lo toque. La posición queda arruinada para siempre
y el enemigo desaparece sin forma de recuperarlo.

**4. Mover:** dirección × velocidad × tiempo, la fórmula de física de toda la
vida. El `+=` suma a la posición actual (no la reemplaza), y por eso el
movimiento es acumulativo.

El enemigo **persigue** aunque el objetivo se mueva porque recalcula todo esto
desde cero, 60 veces por segundo.

**Recibe `Posicion` y no `Jugador`** a propósito: así este módulo no depende del
de los compañeros. Se prueba con un punto fijo, y el día de la integración le
pasan la posición real del jugador sin cambiar una línea.

#### El enemigo vibraba al llegar (overshooting)

Síntoma detectado en la prueba visual: el cuadrado llegaba a la bolita y en vez
de quedarse quieto, **vibraba**.

Causa: el paso por frame es fijo (`velocidad * dt` = 1.92 px a 60 FPS) y no se
achica al acercarse. Cuando falta menos de 1.92 px, el enemigo **se pasa de
largo**; al frame siguiente el objetivo le queda para atrás, da media vuelta, y
se vuelve a pasar. Sesenta veces por segundo.

Solución: antes de moverse, comparar el paso con lo que falta.

```cpp
float paso = velocidad * dt;

if (paso >= distancia) {
    x = objetivo.x;      // llegaria o se pasaria -> lo plantamos ahi
    y = objetivo.y;
} else {
    dx /= distancia;     // camino normal
    dy /= distancia;
    x += dx * paso;
    y += dy * paso;
}
```

> **Ejemplo bobo:** si te falta medio paso para llegar a la puerta, no des un
> paso entero y te choques. Apoyá el pie justo en la puerta.

En el juego terminado casi no se va a ver, porque el enemigo va a chocar contra
el jugador antes de llegar a su centro. Pero si el jugador se queda quieto en un
rincón, vuelve a aparecer — así que conviene tenerlo cubierto.

---

### `obtenerPosicion()`

```cpp
Posicion Enemigo::obtenerPosicion() const {
    Posicion p;
    p.x = x;
    p.y = y;
    return p;
}
```

**No mueve nada: solo informa dónde está el enemigo.** El que mueve es
`actualizar()`.

Es un *getter*. Como `x` e `y` son `private`, nadie de afuera puede leerlos
directamente; este método es la puerta controlada: se puede **leer** la posición,
pero no **cambiarla**.

Devuelve `Posicion` (y no `x` e `y` sueltos) porque una función devuelve un solo
valor, y porque `Posicion` es el tipo que acordaron los tres módulos.

Devuelve una **copia**: quien la reciba puede modificarla y el enemigo ni se
entera.

---

### `recibirDanio(int cantidad)`

```cpp
void Enemigo::recibirDanio(int cantidad) {
    vida = vida - cantidad;
    if (vida < 0) { vida = 0; }
}
```

La **única** forma de bajarle la vida a un enemigo desde afuera.

Si `vida` fuera pública, cualquiera podría escribir `enemigo.vida = 9999;`. Así,
las reglas de cómo cambia la vida viven adentro de la clase. Eso es
**encapsulamiento**, y es lo central de la materia.

El `if` frena la vida en cero para no arrastrar números negativos feos.

---

### `estaVivo()`

```cpp
bool Enemigo::estaVivo() const {
    return vida > 0;
}
```

`vida > 0` ya **es** un `bool`, se devuelve directo. No hace falta
`if (vida > 0) return true; else return false;` — es lo mismo pero más largo.

Se va a usar para sacar del vector los enemigos muertos, y para que el módulo de
colisiones no siga calculando choques contra un muerto.

---

## 6. Cosas de C++ que aparecieron

### `Enemigo::` adelante del método

En el `.h` se **declara** qué sabe hacer la clase; en el `.cpp` se **define**
cómo lo hace. El `Enemigo::` dice "esto es un método de la clase", no una función
suelta. Por eso adentro se puede escribir `x` o `vida` directo: el método ya está
parado adentro del objeto.

### El `const` del final

Promesa al compilador: *"este método no modifica el objeto"*. Si por error
escribís `x = 0;` adentro, no compila.

**Tiene que estar en el `.h` Y en el `.cpp`.** Si está en uno solo, el compilador
los toma como dos métodos distintos y tira un error confuso de "definición no
encontrada".

Los métodos quedan en tres grupos:

| Tipo | Cuáles | ¿`const`? |
|---|---|---|
| Construcción | el constructor | — |
| Cambian el estado | `actualizar`, `recibirDanio` | no |
| Solo consultan | `obtenerPosicion`, `estaVivo` | sí |

Los dos del medio son las **únicas** formas de modificar un enemigo desde afuera:
que pase el tiempo, o que le peguen. Nada más puede alterarlo.

### `private` vs `public`

`private` = solo la clase lo toca. `public` = lo puede usar cualquiera.

> **Ejemplo bobo:** un cajero automático. La plata y el mecanismo están adentro
> (`private`); los botones están afuera (`public`). Podés pedir "dame $1000",
> pero no meter la mano y agarrar lo que quieras.

---

## 7. Las variantes: `enum TipoEnemigo` y `crearEnemigo()`

Las tres variantes **no son tres clases** ni usan herencia: son la misma clase
`Enemigo` con distintos valores en el constructor.

```cpp
enum TipoEnemigo { BASICO, RAPIDO, RESISTENTE };
Enemigo crearEnemigo(TipoEnemigo tipo, float x, float y);   // funcion libre
```

| Variante | id | Tamaño | Vida | Velocidad | Daño | Exp/Chatarra |
|---|---|---|---|---|---|---|
| Básico | 1 | 56×56 | 100 | 120 px/s | 10 | 5 / 3 |
| Rápido | 2 | 40×40 | 60 | 220 px/s | 8 | 7 / 2 |
| Resistente | 3 | 72×72 | 250 | 70 px/s | 15 | 12 / 6 |

**Por qué un `enum` y no un `std::string`:** si escribís `RAPDIO` no compila, te
avisa al toque. Con un string, `"rapdio"` compila perfecto y falla en ejecución.

> **Ejemplo bobo:** son los talles de una remera: S, M o L. No existe el talle
> "banana". El `enum` le dice al compilador cuáles son los valores válidos.

**Por qué `crearEnemigo()` va fuera de la clase:** un método necesita un objeto
que ya exista para poder llamarse, y acá justamente el objeto todavía no existe
— lo estamos creando. Se llama sin objeto: `crearEnemigo(RAPIDO, 100, 100)`.

**Por qué el `default` junto al `case BASICO`:** toda función que devuelve algo
tiene que devolver algo por *todos* los caminos. Sin `default`, el compilador
avisa que podría no retornar nada.

**El `idEnemigo` 1, 2, 3 es la fila del catálogo** en `Enemigos.dat`. Hoy los
valores están escritos en el `switch`; cuando se implemente la persistencia, ese
`switch` se reemplaza por "leé la fila N del archivo" y nada más cambia.

---

## 8. Muchos enemigos: el `vector`

El patrón de cada fotograma, **en este orden**:

```
1. generar los que correspondan
2. actualizar todos          (un for con &)
3. limpiar los muertos       (pasada APARTE)
4. dibujar los que quedan
```

### El `&` del `for` no es opcional

```cpp
for (Enemigo& e : enemigos)   // bien: trabaja sobre el original
for (Enemigo e : enemigos)    // mal: trabaja sobre una copia
```

Sin el `&`, C++ entrega una **copia** de cada enemigo, se mueve la copia y el
original queda quieto. No da error ni warning: simplemente no pasa nada en
pantalla. Es el error que más tiempo hace perder con vectores.

### Nunca borrar mientras se recorre

La tentación:

```cpp
for (size_t i = 0; i < v.size(); i++) {
    if (!v[i].estaVivo()) { v.erase(v.begin() + i); }   // MAL
}
```

Al borrar, los de atrás se corren un lugar, pero el `i++` avanza igual: **te
salteás uno por cada borrado**. Comprobado con 5 enemigos, matando los 3
primeros — la respuesta correcta es 2 y este código deja 3.

Y con el `for (Enemigo& e : v)` es peor: borrar ahí adentro corrompe el
recorrido. Es *undefined behavior*: a veces anda de casualidad y te oculta el
bug hasta la demo.

### La forma correcta

```cpp
enemigos.erase(
    std::remove_if(enemigos.begin(), enemigos.end(),
                   [](const Enemigo& e) { return !e.estaVivo(); }),
    enemigos.end());
```

`remove_if` **no borra**: empuja los que cumplen la condición hacia el final y
devuelve dónde arranca la "zona de basura". El `erase` corta desde ahí. Necesita
`#include <algorithm>`.

Eso `[](const Enemigo& e) { ... }` es una **lambda**: una función anónima escrita
en el lugar donde se usa, para no declarar una función aparte solo para esto.

---

## 9. `GestorOleadas`: la dificultad por tiempo

Encapsula todo el módulo detrás de una clase. Quien la usa solo llama a
`actualizar()` una vez por fotograma y recorre `obtenerEnemigos()` para dibujar.
No tiene que acordarse del orden de las fases, ni del `&`, ni del `erase`.

```cpp
void actualizar(Posicion posicionJugador, float dt);  // genera, mueve, limpia
const std::vector<Enemigo>& obtenerEnemigos() const;
void reiniciar();
float tiempoDePartida() const;
int oleadaActual() const;
std::size_t maximoVivosActual() const;
```

### Aparecen en un anillo, no en los bordes

Al principio pensé "desde los bordes de la pantalla", pero con la cámara que
agregó Lucas el mapa es más grande que la ventana: los bordes de la pantalla ya
no son los bordes del mundo.

La solución del género es un **anillo alrededor del jugador**: ángulo al azar, a
700 px de distancia.

```cpp
x = jugador.x + cos(angulo) * 700;
y = jugador.y + sin(angulo) * 700;
```

`cos` y `sin` convierten un ángulo en una dirección — es la operación inversa a
la de `actualizar()`, donde de una dirección sacábamos la distancia. Ángulo 0 es
a la derecha, 90° arriba, 180° a la izquierda.

Así aparecen rodeando al jugador, siempre fuera de la vista, nunca encima.

### Las oleadas son tramos de tiempo, no grupos

La oleada 1 son los primeros 30 segundos, la 2 los siguientes 30, y así. **No
hay que matarlos a todos para pasar a la siguiente.**

El motivo: con oleadas cerradas, un enemigo escondido en una esquina del mapa
traba la partida para siempre. Por tiempo eso no puede pasar. Es también lo que
pidió Lucas.

### Todo sale de una sola fórmula

`progresoDificultad()` va de 0 a 1 en cinco minutos y después se queda en 1. El
intervalo y el techo se **interpolan** con ese número:

```cpp
intervalo = 1.5 - (1.5 - 0.4) * progreso;      // baja de 1.5s a 0.4s
tope      = 15  + (60  - 15 ) * progreso;      // sube de 15 a 60
```

Interpolar es "dame el valor intermedio": con progreso 0.5, el intervalo queda
justo en el medio entre 1.5 y 0.4.

Simulación de seis minutos, sin matar a nadie:

```
 min | oleada | intervalo | tope | vivos | basicos rapidos resist
   0 |      1 |      1.50 |   15 |     0 |       0       0      0
   1 |      3 |      1.28 |   24 |    23 |      23       0      0
   2 |      5 |      1.06 |   33 |    32 |      26       6      0
   3 |      7 |      0.84 |   42 |    41 |      29      12      0
   4 |      9 |      0.62 |   51 |    50 |      33      15      2
   5 |     11 |      0.40 |   60 |    60 |      36      20      4
```

### El techo de vivos es lo más importante

Sin ese límite el vector crece sin parar y a los pocos minutos el juego se
arrastra: cada fotograma hay que mover y dibujar a todos. Es un `if` antes de
generar.

Detalle: el temporizador corre igual aunque el techo esté lleno; lo que se
descarta es esa aparición puntual.

### Las variantes se habilitan con el tiempo

| Desde | Qué aparece |
|---|---|
| segundo 0 | básico |
| segundo 60 | + rápido |
| segundo 180 | + resistente |

Entre las habilitadas se sortea con la misma probabilidad. Así la dificultad no
es solo "más enemigos": cambia el tipo de amenaza.

### La semilla del azar está fija

`generador(20261005u)` — a propósito, mientras probamos: si algo sale mal, se
reproduce igual. Para que cada partida sea distinta se cambia por
`std::random_device{}()`, una línea.

### La corrección del centro vive acá

El gestor le pasa a cada enemigo un objetivo ya corregido:

```cpp
objetivo.x = posicionJugador.x - forma.ancho / 2.0f;
```

Sin esto el enemigo persigue con su esquina superior izquierda y queda siempre
corrido medio cuerpo. Lucas hacía esta misma cuenta en `Juego.cpp`; al integrar
el gestor la borró, porque ya está resuelta adentro.

---

## 10. El jefe final

Es la condición de victoria: hasta que existió, se podía sobrevivir para
siempre pero no ganar.

**No es una clase nueva.** Es una cuarta variante del `enum`, con `idEnemigo`
4 — su fila en `Enemigos.dat`. Lo único distinto es cuándo y cómo aparece.

| | |
|---|---|
| Aparece | minuto 5, cuando la dificultad llega a su máximo |
| Aviso | 5 segundos antes |
| Tamaño | 120×120 |
| Vida | 3000 |
| Velocidad | 85 px/s |
| Recompensa | 200 exp, 120 chatarra |

### Tres decisiones que no son obvias

**No pasa por el techo de enemigos vivos.** Si el techo estuviera lleno de
comunes, el jefe nunca saldría y la partida sería imposible de ganar. Se genera
aparte del temporizador normal.

**Mientras vive, los comunes aparecen al doble de intervalo.** Pelear contra el
jefe con el ritmo máximo de oleadas encima hacía la victoria impracticable.

**Se lo reconoce por `obtenerIdInstancia()`, no por su posición en el vector**,
porque el vector se reordena cada vez que se limpian los muertos.

### El orden importa (otra vez)

```cpp
detectarJefeDerrotado();   // primero
quitarMuertos();           // despues
```

Una vez que el jefe sale del vector ya no se puede distinguir si murió o si
nunca estuvo. Por eso se revisa antes de limpiar.

### Lo que el gestor informa

```cpp
bool jefeEnCamino() const;        // mostrá el aviso
float segundosParaElJefe() const; // para una cuenta regresiva
bool jefeEstaVivo() const;        // está en el mapa
bool jefeFueDerrotado() const;    // ← LA VICTORIA
```

El gestor no dibuja nada: solo informa en qué momento está la partida. Quien
dibuja decide cómo mostrarlo.

---

## 11. `obtenerIdInstancia()`: identificar a cada enemigo

Distinto de `obtenerIdEnemigo()`, que dice la **variante** (1, 2, 3, 4). Este
identifica a **ese enemigo puntual**: 1, 2, 3… y nunca se repite.

```cpp
static std::uint64_t siguienteIdInstancia;          // en el .h, private
std::uint64_t Enemigo::siguienteIdInstancia = 1;    // en el .cpp
```

**Qué significa `static` en una clase:** la variable es **una sola, compartida
por todos los enemigos** — no pertenece a ninguno. Cada enemigo que se
construye se lleva el número actual y lo incrementa.

> **Ejemplo bobo:** es el rollito de números del Registro Civil. El rollo es uno
> solo; cada persona que llega arranca un número y el siguiente queda para el
> que viene.

**Arranca en 1 y no en 0** porque `Proyectil::registrarImpacto()` descarta el
id 0: un enemigo con ese número nunca recibiría impactos del cañón.

### Para qué hizo falta

El cañón de energía atraviesa varios enemigos y no debe pegarle dos veces al
mismo. Antes, `Juego` mantenía un vector de identificadores en paralelo al de
enemigos y lo alineaba a mano, asumiendo que el gestor agregaba al final y
preservaba el orden al limpiar. Funcionaba, pero cualquier cambio interno del
gestor lo habría roto en silencio. Con el id adentro del enemigo, ese vector
paralelo desapareció.

---

## 12. Los archivos binarios

Dos archivos, con formas distintas.

| Archivo | Qué es | Modo |
|---|---|---|
| `Enemigos.dat` | catálogo de variantes, se lee al empezar | `"wb"` / `"rb"` |
| `Partidas.dat` | historial, crece con cada partida | `"ab"` para agregar |

### Por qué structs planos

Solo `int`, `float`, `bool` y arreglos de `char`. **Nada de `std::string`**: un
string guarda internamente un puntero a otra parte de la memoria, y volcarlo al
archivo guardaría una dirección que mañana no significa nada.

### `Enemigos.dat`: el catálogo manda

El gestor ya **no** toma las estadísticas de las constantes escritas en
`crearEnemigo()`: las lee del archivo al construirse. Se puede rebalancear el
juego sin recompilar.

Comprobado cambiando la vida del básico a 777 en el archivo: los enemigos
generados aguantaron exactamente 777 golpes.

Si el archivo falta, o ningún registro pasa la validación, se rehace con los
valores de fábrica. Así la primera ejecución en una máquina nueva funciona sin
pasos previos.

El registro tiene los 7 campos del informe **más `ancho` y `alto`**: sin ellos
el catálogo no alcanza para construir un enemigo. *(Pendiente: actualizar esa
línea del informe.)*

### `Partidas.dat`: `"ab"` en vez de `"wb"`

```cpp
FILE* archivo = fopen(RUTA, "ab");   // agrega al final
```

Con `"wb"` cada partida pisaría el historial entero. El número de registro sale
de cuántos había antes.

Una partida **en curso se rechaza**: su duración y su resultado todavía pueden
cambiar.

### Leer sin cargar basura

```cpp
while (fread(&registro, sizeof(RegistroPartida), 1, archivo) == 1) {
```

`fread` devuelve cuántos registros **completos** leyó. Si el archivo quedó
cortado por la mitad, el pedazo incompleto se descarta en vez de entrar como
basura.

### El bug de la carpeta `datos/`

**`fopen` no crea carpetas.** Si no existe `datos/`, devuelve `NULL` y el
guardado se pierde sin ningún aviso.

Eso pasaba al ejecutar el juego con doble clic sobre `bin\Debug\ProyectoTPI.exe`:
la ruta `"datos/..."` se resuelve contra el **directorio de trabajo**, que ahí
es `bin\Debug` y no la raíz del proyecto.

| | Antes | Ahora |
|---|---|---|
| Desde Code::Blocks | guardaba | guarda |
| Doble clic en el `.exe` | **no guardaba nada** | guarda |

La solución está en `CarpetaDatos.h`: `asegurarCarpetaDatos()` llama a `_mkdir`
antes de cada escritura. Se usa `_mkdir` y no `std::filesystem` porque este
último compila pero el ejecutable no arranca con este MinGW.

### Qué se versiona y qué no

- `Enemigos.dat` **sí**: es contenido del juego.
- `Partidas.dat` **no** (está en el `.gitignore`): lo genera cada uno al jugar,
  y versionarlo daría conflictos binarios en cada partida.

---

## 13. Reglas de trabajo del grupo

- Archivos **míos**: `Enemigo`, `GestorOleadas`, `Partida`, `ArchivoEnemigos`,
  `ArchivoPartidas`, `CarpetaDatos.h` y las pruebas.
- Archivos que **no toco sin avisar**: `Juego.*`, `main.cpp`, `Configuracion.h`,
  `TiposCompartidos.h`.
- `ProyectoTPI.cbp` lo van a modificar los tres al agregar archivos → es el
  conflicto más probable del TPI. Es XML, se resuelve a mano leyéndolo.
- Cada paso tiene que **compilar y demostrarse** antes de empezar el siguiente.

---

## 14. Estado del módulo

Terminado:

- [x] `Enemigo` — persecución con delta time, daño, vida, recompensa
- [x] Tres variantes + el jefe (`enum TipoEnemigo` y `crearEnemigo`)
- [x] `obtenerIdInstancia()` — identidad única por enemigo
- [x] `GestorOleadas` — aparición en anillo, techo de vivos, limpieza
- [x] Dificultad progresiva por tiempo y numeración de oleadas
- [x] Jefe final en el minuto 5, con aviso y condición de victoria
- [x] `Partida` — puntaje, duración, bajas, victoria/derrota
- [x] `ArchivoEnemigos` — catálogo en `datos/Enemigos.dat`
- [x] `ArchivoPartidas` — historial en `datos/Partidas.dat`
- [x] `pruebas/prueba_enemigos.cpp` + `compilar_pruebas.bat`

Pendiente, pero no es código mío:

- [ ] Que `Juego` llame a `ArchivoPartidas::agregar(partida)` al terminar la
      partida, donde ya detecta victoria y derrota. Es una línea.
- [ ] Actualizar el informe: `Enemigos.dat` pasó de 7 a 9 campos (se le
      sumaron `ancho` y `alto`).
- [ ] Avisar que `CarpetaDatos.h` también le sirve a `ArchivoJugadores` y
      `ArchivoMejoras`, que tienen el mismo problema de la carpeta.
- [ ] `Armas.dat` es el único de los cinco binarios que falta (es de Lucas).
