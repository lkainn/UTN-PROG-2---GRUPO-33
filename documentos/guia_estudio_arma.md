# Guia de estudio de la clase Arma - Proyecto TPI

Esta guia explica la clase `Arma`, implementada durante esta sesion, y las
interacciones necesarias con los otros modulos del Proyecto TPI. El objetivo es
entender el codigo real del proyecto y poder explicarlo durante una revision o
defensa oral.

Archivos principales:

- [`Arma.h`](../include/Arma.h) y [`Arma.cpp`](../src/Arma.cpp).
- [`Juego.h`](../include/Juego.h) y [`Juego.cpp`](../src/Juego.cpp).
- [`Proyectil.h`](../include/Proyectil.h) y
  [`Proyectil.cpp`](../src/Proyectil.cpp).
- [`Enemigo.h`](../include/Enemigo.h) y [`Enemigo.cpp`](../src/Enemigo.cpp).
- [`SistemaColisiones.h`](../include/SistemaColisiones.h) y
  [`SistemaColisiones.cpp`](../src/SistemaColisiones.cpp).

## 1. Responsabilidad de Arma

La clase `Arma` representa las estadisticas y el comportamiento necesario para
crear un disparo.

Sus responsabilidades son:

- Guardar la identificacion y las estadisticas del arma.
- Controlar cuanto tiempo falta para volver a disparar.
- Comprobar si el objetivo esta dentro del alcance.
- Calcular la direccion del disparo.
- Crear un `Proyectil` con el dano correspondiente.
- Evitar que un proyectil activo sea reemplazado.

`Arma` no mueve el proyectil, no busca enemigos, no detecta colisiones y no
reduce directamente la vida de un enemigo.

Esta separacion evita que una sola clase concentre toda la logica de combate.

## 2. Atributos

La clase contiene:

```cpp
int _id;
char _nombre[30];
int _dano;
float _velocidadAtaque;
float _alcance;
int _nivelActual;
int _nivelMaximo;
float _tiempoRestante;
```

### `_id`

Identifica el tipo de arma. Mas adelante puede relacionarse con un registro de
`Armas.dat`.

### `_nombre[30]`

Guarda el nombre en un arreglo fijo de caracteres. Se usa `char[30]` porque es
compatible con el enfoque de archivos binarios de la materia.

### `_dano`

Es el dano que recibe el proyectil creado por el arma. El arma no aplica este
dano directamente al enemigo.

### `_velocidadAtaque`

Representa cuantos ataques puede realizar por segundo. No es la velocidad de
movimiento del proyectil.

### `_alcance`

Es la distancia maxima permitida entre el origen del disparo y el objetivo.

### `_nivelActual` y `_nivelMaximo`

Preparan al arma para un futuro sistema de mejoras. Actualmente el nivel se
guarda, pero todavia no modifica sus estadisticas.

### `_tiempoRestante`

Es el tiempo que falta para volver a disparar. Es un estado temporal de la
partida y no una estadistica de catalogo.

## 3. Constructor y validaciones

La declaracion del constructor es:

```cpp
Arma(int id, const char* nombre, int dano, float velocidadAtaque,
     float alcance, int nivelMaximo);
```

La lista de inicializacion asigna valores a todos los atributos antes de entrar
al cuerpo del constructor.

El constructor valida:

- ID mayor que cero.
- Dano mayor que cero.
- Velocidad de ataque finita y mayor que cero.
- Alcance finito y mayor que cero.
- Nivel maximo mayor que cero.
- Nombre diferente de `nullptr`.

Cuando un dato numerico no es valido, se guarda `0`. De esa manera el arma queda
inhabilitada en lugar de producir un comportamiento indefinido.

El nombre se copia asi:

```cpp
std::strncpy(_nombre, nombre, sizeof(_nombre) - 1);
_nombre[sizeof(_nombre) - 1] = '\0';
```

Se reserva la ultima posicion para `\0`, que indica el final de una cadena de
caracteres de C.

## 4. Cadencia y cooldown

La velocidad de ataque se expresa en ataques por segundo.

El arma de prueba utiliza:

```cpp
velocidadAtaque = 2.0f;
```

Por lo tanto, el intervalo entre disparos es:

```text
1 / 2 = 0,5 segundos
```

Despues de disparar, el arma ejecuta:

```cpp
_tiempoRestante = 1.0f / _velocidadAtaque;
```

En cada fotograma, `actualizar()` descuenta el tiempo transcurrido:

```cpp
_tiempoRestante = std::max(0.0f, _tiempoRestante - segundos);
```

`std::max` evita que el contador quede por debajo de cero.

El primer disparo puede producirse inmediatamente porque el constructor inicia:

```cpp
_tiempoRestante(0.0f)
```

El metodo:

```cpp
bool puedeDisparar() const;
```

comprueba que:

- El dano sea valido.
- La velocidad de ataque sea valida.
- El cooldown haya terminado.

## 5. Metodo intentarDisparar

La firma actual es:

```cpp
bool intentarDisparar(
    Proyectil& proyectil,
    Posicion origen,
    Posicion objetivo,
    float velocidadProyectil,
    float anchoProyectil,
    float altoProyectil
);
```

Devuelve `true` cuando consigue crear el disparo y `false` cuando alguna
condicion lo impide.

El procedimiento general es:

1. Comprobar el cooldown.
2. Comprobar que no exista otro proyectil activo.
3. Validar la velocidad del proyectil.
4. Calcular la distancia hasta el objetivo.
5. Comprobar el alcance.
6. Normalizar la direccion.
7. Crear un proyectil temporal.
8. Verificar que el proyectil sea valido.
9. Copiarlo al proyectil administrado por `Juego`.
10. Reiniciar el cooldown.

## 6. Paso por referencia

El parametro se declara como:

```cpp
Proyectil& proyectil
```

El simbolo `&` indica que se recibe una referencia al objeto original. Por eso
esta asignacion modifica el proyectil conservado en `Juego`:

```cpp
proyectil = nuevoProyectil;
```

Si se recibiera por valor:

```cpp
Proyectil proyectil
```

se modificaria una copia local. Esa copia desapareceria al terminar el metodo y
`Juego` seguiria teniendo el proyectil anterior.

## 7. Calculo de la direccion

La diferencia entre el objetivo y el origen produce un vector:

```cpp
const float diferenciaX = objetivo.x - origen.x;
const float diferenciaY = objetivo.y - origen.y;
```

Ejemplo:

```text
Origen:   (100, 100)
Objetivo: (400, 500)

diferenciaX = 300
diferenciaY = 400
```

La distancia se calcula con:

```cpp
const float distancia = std::hypot(diferenciaX, diferenciaY);
```

En el ejemplo:

```text
distancia = sqrt(300² + 400²)
distancia = 500
```

## 8. Normalizacion

Normalizar un vector significa convertirlo a longitud `1` sin cambiar su
direccion.

```cpp
const float velocidadX =
    diferenciaX / distancia * velocidadProyectil;
const float velocidadY =
    diferenciaY / distancia * velocidadProyectil;
```

Si la velocidad total es `100`:

```text
velocidadX = 300 / 500 * 100 = 60
velocidadY = 400 / 500 * 100 = 80
```

La velocidad total continua siendo `100`. Sin normalizacion, los proyectiles
diagonales se moverian mas rapido que los horizontales.

## 9. Division por cero y valores no finitos

Si origen y objetivo coinciden:

```text
diferenciaX = 0
diferenciaY = 0
distancia = 0
```

No se puede dividir por esa distancia. Por eso se valida:

```cpp
if (!std::isfinite(distancia) || distancia <= 0.0f ||
    distancia > _alcance) {
    return false;
}
```

`std::isfinite` rechaza infinito y `NaN`.

Si el intento se rechaza, el cooldown no se consume.

## 10. Posicion inicial del proyectil

El origen recibido representa el centro del jugador. La posicion de
`Proyectil`, en cambio, representa su esquina superior izquierda.

La conversion es:

```cpp
const Posicion posicionInicial{
    origen.x - anchoProyectil / 2.0f,
    origen.y - altoProyectil / 2.0f
};
```

Restar la mitad de cada dimension coloca el centro del proyectil sobre el centro
del jugador.

## 11. Alcance del arma

La ventana mide `1920 x 1080`. Para llegar desde una esquina hasta la contraria,
el alcance debe cubrir la diagonal:

```text
sqrt(1920² + 1080²) = 2202,91 pixeles aproximadamente
```

La configuracion temporal del arma usa:

```cpp
constexpr float ALCANCE_ARMA = 2210.0f;
```

El valor es apenas mayor que la diagonal y permite disparar hacia cualquier
posicion dentro de la ventana.

## 12. Interaccion con Juego

`Juego` coordina el sistema. La clase `Arma` no conoce el arreglo de enemigos ni
decide por si sola a cual atacar.

`Juego::actualizarDisparoAutomatico()` realiza estas tareas:

1. Comprueba que no exista un proyectil activo.
2. Comprueba que el arma pueda disparar.
3. Obtiene el centro del jugador.
4. Recorre los enemigos vivos.
5. Selecciona el centro del enemigo mas cercano.
6. Llama a `armaPrueba.intentarDisparar()`.
7. Sincroniza la figura grafica si el disparo fue creado.

La seleccion del objetivo pertenece a `Juego` porque alli se encuentran y se
coordinan todas las entidades de la demostracion.

### 12.1 Por que se usa `std::array`

`Juego` necesita administrar varios enemigos, pero la cantidad de esta
demostracion es fija y conocida al compilar. Por eso se usa `std::array` en vez
de memoria dinamica:

```cpp
static constexpr std::size_t CANTIDAD_ENEMIGOS = 3;
```

`std::array` guarda una cantidad fija de elementos del mismo tipo. En este caso
siempre existen tres lugares disponibles para enemigos, figuras y tiempos de
reaparicion.

La constante evita escribir el numero `3` en todas las declaraciones. Si la
cantidad cambia, los arreglos que usan `CANTIDAD_ENEMIGOS` cambian juntos.

### 12.2 Arreglo de posiciones iniciales

En `Juego.cpp` se declaran las posiciones donde aparecen los enemigos:

```cpp
constexpr std::array<Posicion, 3> POSICIONES_INICIALES_ENEMIGOS{{
    {1440.0f, 200.0f},
    {1440.0f, 512.0f},
    {1440.0f, 824.0f}
}};
```

Cada elemento es una estructura `Posicion` con una coordenada `x` y una
coordenada `y`:

```text
Indice 0 → posicion superior derecha
Indice 1 → posicion central derecha
Indice 2 → posicion inferior derecha
```

El arreglo es `constexpr` porque estas posiciones son datos fijos de la
demostracion. No cambian durante la ejecucion.

La funcion `crearEnemigo()` recibe un indice y consulta la posicion
correspondiente:

```cpp
Enemigo crearEnemigo(std::size_t indice) {
    const Posicion posicion = POSICIONES_INICIALES_ENEMIGOS[indice];

    return Enemigo(
        static_cast<int>(indice) + 1,
        posicion.x,
        posicion.y,
        // otras estadisticas
    );
}
```

El indice tambien se usa para generar un identificador desde `1`. El elemento
`0` recibe ID `1`, el elemento `1` recibe ID `2` y el elemento `2` recibe ID
`3`.

### 12.3 Arreglos relacionados por indice

`Juego` conserva tres arreglos principales:

```cpp
std::array<Enemigo, CANTIDAD_ENEMIGOS> enemigosPrueba;

std::array<sf::RectangleShape, CANTIDAD_ENEMIGOS>
    figurasEnemigosPrueba;

std::array<float, CANTIDAD_ENEMIGOS> tiemposReaparicion;
```

Estos son arreglos paralelos. Los elementos que tienen el mismo indice
pertenecen a la misma entidad:

```text
Indice i
├── enemigosPrueba[i]          → estado logico del enemigo
├── figurasEnemigosPrueba[i]   → representacion visual
├── tiemposReaparicion[i]      → tiempo restante si esta muerto
└── POSICIONES_INICIALES_ENEMIGOS[i] → lugar donde reaparece
```

Por ejemplo, para el enemigo del indice `1` se deben utilizar:

```cpp
enemigosPrueba[1];
figurasEnemigosPrueba[1];
tiemposReaparicion[1];
POSICIONES_INICIALES_ENEMIGOS[1];
```

Mezclar indices produciria errores. Por ejemplo, mover
`figurasEnemigosPrueba[2]` usando la posicion de `enemigosPrueba[0]` haria que
la figura visible no coincida con el enemigo logico.

### 12.4 Inicializacion de los arreglos

`Enemigo` no tiene un constructor sin parametros. Por eso los tres elementos se
construyen explicitamente en la lista de inicializacion de `Juego`:

```cpp
enemigosPrueba{
    crearEnemigo(0),
    crearEnemigo(1),
    crearEnemigo(2)
}
```

Las figuras tambien se construyen con su tamaño:

```cpp
figurasEnemigosPrueba{
    sf::RectangleShape({ANCHO_ENEMIGO, ALTO_ENEMIGO}),
    sf::RectangleShape({ANCHO_ENEMIGO, ALTO_ENEMIGO}),
    sf::RectangleShape({ANCHO_ENEMIGO, ALTO_ENEMIGO})
}
```

Los temporizadores se inicializan con llaves vacias:

```cpp
tiemposReaparicion{}
```

En un `std::array<float, 3>`, esta inicializacion coloca los tres valores en
`0.0f`.

### 12.5 Recorrido de los arreglos

Los arreglos se recorren con un ciclo `for`:

```cpp
for (std::size_t i = 0; i < CANTIDAD_ENEMIGOS; i++) {
    // usar enemigosPrueba[i]
}
```

`std::size_t` es un tipo entero sin signo utilizado para tamaños e indices de
colecciones.

El mismo tipo de recorrido se usa para:

- Actualizar la posicion de cada enemigo vivo.
- Descontar el tiempo de reaparicion de cada enemigo muerto.
- Comparar el proyectil contra todos los enemigos.
- Dibujar las figuras de los enemigos vivos.
- Reiniciar los tres enemigos al presionar `R`.
- Buscar el objetivo mas cercano para el arma.

### 12.6 Como participan los arreglos en el disparo

El arma no recibe el arreglo completo. `Juego` lo recorre y filtra los enemigos
muertos:

```cpp
for (std::size_t i = 0; i < CANTIDAD_ENEMIGOS; i++) {
    if (!enemigosPrueba[i].estaVivo()) {
        continue;
    }

    // calcular la distancia al enemigo vivo
}
```

`continue` salta el resto de esa repeticion y pasa al siguiente indice.

Para cada enemigo vivo, `Juego` calcula la distancia al cuadrado:

```cpp
const float distanciaCuadrada =
    diferenciaX * diferenciaX + diferenciaY * diferenciaY;
```

No hace falta obtener la raiz cuadrada para comparar cual esta mas cerca. El
menor valor de `distanciaCuadrada` tambien representa la menor distancia real.

Cuando termina el recorrido, `Juego` conserva un solo `centroObjetivo` y se lo
entrega a `Arma::intentarDisparar()`.

La division de responsabilidades queda asi:

```text
Juego recorre el arreglo y selecciona un enemigo
                         ↓
Arma recibe solamente la posicion del objetivo elegido
                         ↓
Arma valida el alcance y crea el proyectil
```

De esta manera, `Arma` puede funcionar sin conocer cuantos enemigos existen ni
como estan almacenados.

## 13. Interaccion con Proyectil

`Arma` crea un `Proyectil` con:

- Posicion inicial.
- Velocidad horizontal.
- Velocidad vertical.
- Ancho y alto.
- Dano del arma.

Despues de crearlo, `Arma` deja de intervenir en su movimiento.

`Proyectil::actualizar()` aplica:

```cpp
_posicion.x += _velocidadX * segundos;
_posicion.y += _velocidadY * segundos;
```

Esta es la formula:

```text
distancia recorrida = velocidad * tiempo
```

El proyectil actual no es teledirigido. Conserva la direccion calculada al
momento del disparo, aunque el enemigo cambie de posicion.

## 14. Interaccion con Enemigo

`Arma` no recibe un objeto `Enemigo`. Recibe solamente la posicion del objetivo.

Esto reduce el acoplamiento: `Arma` no necesita conocer los atributos privados
ni los metodos internos de `Enemigo`.

`Juego` obtiene la forma del enemigo, calcula su centro y entrega esa posicion al
arma.

Cuando el proyectil impacta, `Juego` ejecuta:

```cpp
enemigosPrueba[i].recibirDanio(proyectilPrueba.obtenerDano());
```

El dano fue definido por `Arma`, transportado por `Proyectil` y finalmente
aplicado por `Juego` sobre `Enemigo`.

## 15. Interaccion con SistemaColisiones

`Arma` tampoco consulta las colisiones.

`SistemaColisiones` compara la forma del proyectil contra la forma de cada
enemigo. Si informa un impacto, `Juego` aplica el dano y desactiva el proyectil.

Esta separacion permite utilizar el mismo sistema de colisiones con otras
entidades sin incluir logica especifica de armas.

## 16. Flujo completo del disparo

```text
Juego selecciona un objetivo vivo
            ↓
Juego entrega origen y objetivo a Arma
            ↓
Arma valida cooldown, alcance y proyectil activo
            ↓
Arma calcula la direccion y crea Proyectil
            ↓
Proyectil actualiza su posicion cada fotograma
            ↓
SistemaColisiones informa un impacto
            ↓
Juego obtiene el dano del proyectil
            ↓
Enemigo recibe el dano
```

Cada modulo participa sin apropiarse de las responsabilidades de los demas.

## 17. Orden dentro del fotograma

Las operaciones relacionadas con el arma se realizan en este orden:

1. Descontar el cooldown con `armaPrueba.actualizar(segundos)`.
2. Actualizar las posiciones de jugador y enemigos.
3. Mover el proyectil existente.
4. Resolver su impacto o salida del escenario.
5. Si ya no hay proyectil, seleccionar un objetivo.
6. Intentar crear el siguiente disparo.

El disparo automatico se evalua despues de resolver el proyectil anterior. Asi
no se elige como objetivo a un enemigo que acaba de morir en ese fotograma.

## 18. Metodos de consulta `const`

Los metodos de consulta terminan en `const`:

```cpp
int obtenerDano() const;
float obtenerAlcance() const;
bool puedeDisparar() const;
```

Ese `const` indica que el metodo no puede modificar los atributos del objeto.

Tambien comunica la intencion: el metodo solamente consulta informacion.

## 19. Reinicio del arma

La tecla `R` reinicia la demostracion. Para que el arma pueda disparar
inmediatamente se llama:

```cpp
armaPrueba.reiniciar();
```

El metodo coloca:

```cpp
_tiempoRestante = 0.0f;
```

No modifica las estadisticas del arma; solamente reinicia su estado temporal.

## 20. Errores que este diseño evita

### Sobrescribir un proyectil activo

Se comprueba `proyectil.estaActivo()` antes de crear otro.

### Consumir cooldown con datos invalidos

El cooldown se reinicia unicamente despues de construir un proyectil valido.

### Dividir por cero

Se rechaza una distancia menor o igual que cero.

### Diferente velocidad en diagonal

La direccion se normaliza antes de multiplicarla por la velocidad.

### Aplicar dano desde Arma

El arma solo coloca su dano dentro del proyectil. `Juego` resuelve el impacto.

### Conocer los detalles internos de Enemigo

El arma recibe una `Posicion`, no un objeto `Enemigo`.
