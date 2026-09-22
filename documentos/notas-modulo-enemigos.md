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

## 3. `Comunes.h` — el vocabulario entre módulos

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

## 7. Reglas de trabajo del grupo

- Archivos **míos**: `Enemigo`, `Partida`, `ArchivoPartidas`, y las pruebas.
- Archivos que **no toco sin avisar**: `Juego.*`, `main.cpp`, `Configuracion.h`,
  `Comunes.h`.
- `ProyectoTPI.cbp` lo van a modificar los tres al agregar archivos → es el
  conflicto más probable del TPI. Es XML, se resuelve a mano leyéndolo.
- Cada paso tiene que **compilar y demostrarse** antes de empezar el siguiente.

---

## 8. Estado del módulo

- [x] `Comunes.h` con las tres structs
- [x] `Enemigo.h` — atributos y contrato
- [x] `Enemigo.cpp` — constructor, `actualizar`, getters
- [ ] `pruebas/prueba_enemigos.cpp` — verlo moverse en pantalla
- [ ] Variantes: básico, rápido, resistente
- [ ] Oleadas y dificultad progresiva
- [ ] Jefe final y clase `Partida`
- [ ] `Enemigos.dat` y `Partidas.dat`
