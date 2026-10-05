# Guía de recolectables: cofres y estaciones - Proyecto TPI


Archivos principales:

- [EncuentroAleatorio.h](../include/EncuentroAleatorio.h) y
  [EncuentroAleatorio.cpp](../src/EncuentroAleatorio.cpp).
- [EstacionMejoras.h](../include/EstacionMejoras.h) y
  [EstacionMejoras.cpp](../src/EstacionMejoras.cpp).
- [Juego.h](../include/Juego.h) y [Juego.cpp](../src/Juego.cpp).
- [Arma.cpp](../src/Arma.cpp) y [Jugador.h](../include/Jugador.h).
- [Pruebas de encuentros](../pruebas/Encuentros.cpp).
- [Pruebas de estaciones](../pruebas/EstacionMejoras.cpp).

Guías relacionadas: [escenario](guia_escenario.md),
[proyectiles](guia_proyectiles_texturas.md) y
[daño, recompensas e interfaz](guia_estudio_danio_recompensas_interfaz.md).

## 1. Diferencia respecto de los recursos del suelo

La experiencia y la chatarra que dejan los enemigos se recogen automáticamente
por cercanía. Cofres y estaciones requieren una interacción con F y un pago.
No se administran con la misma colección de recursos del suelo.

| Parte | Responsabilidad |
|---|---|
| `EncuentroAleatorio` | Genera zonas y conserva posición, presencia y uso de objetos. |
| `EstacionMejoras` | Conserva precio global de estaciones, menú y bonos generales. |
| `Juego` | Coordina búsqueda, cobro, recompensa, dibujo y estado de la compra. |
| `Jugador` | Informa saldo y descuenta chatarra mediante su interfaz. |
| `Arma` | Aplica las bonificaciones recibidas a daño y cadencia. |

El nombre `EncuentroAleatorio` quedó de la primera versión. Actualmente no
administra apariciones por tiempo: genera un mapa de objetos persistentes.

## 2. Estructura de un objeto

```cpp
enum class TipoEncuentro
{
    Estacion,
    Cofre
};

struct ObjetoEncuentro
{
    TipoEncuentro tipo;
    Posicion posicion;
    bool usado;
    bool presente = true;
};
```

`enum class` define valores con nombre. Se escribe `TipoEncuentro::Cofre`
en lugar de un número que obligue a recordar su significado.

`presente` y `usado` tienen significados distintos:

| Presente | Usado | Estado |
|---|---|---|
| false | false | No apareció ningún objeto en ese lugar. |
| true | false | Objeto disponible para interactuar. |
| true | true | Objeto visible, pero ya consumido. |

Un cofre abierto no se elimina: cambia su atributo `usado`.

## 3. using, array, pair y map

```cpp
using Zona = std::array<ObjetoEncuentro, 3>;
using Zonas = std::map<std::pair<int, int>, Zona>;
```

`using` da un nombre alternativo a un tipo; no crea un objeto ni una clase nueva.
Escribir `Zona zona;` equivale a declarar un arreglo de tres `ObjetoEncuentro`.

`std::array` tiene tamaño fijo. Sus tres espacios son una ubicación posible
de estación y dos ubicaciones posibles de cofres. No necesariamente aparecen todos.

`std::pair<int, int>` agrupa dos enteros: coordenada X e Y de una zona.
`std::map` relaciona cada pareja de coordenadas con su `Zona`.

Ejemplo didáctico:

```cpp
Zonas zonas;
Zona zona{};
zonas.emplace(std::make_pair(2, -1), zona);
```

`emplace` inserta la zona bajo la clave `(2, -1)`. El ejemplo ilustra los tipos;
el juego construye cada zona con posiciones y estados concretos.

## 4. Coordenadas de zona y generación anticipada

```cpp
constexpr double TAMANIO_ZONA = 1200.0;
constexpr int RADIO_GENERACION = 2;
```

Cada zona ocupa 1200 × 1200 píxeles. La zona del jugador se calcula así:

```cpp
const double zonaX = std::floor(posicion.x / TAMANIO_ZONA);
const double zonaY = std::floor(posicion.y / TAMANIO_ZONA);
```

En X = 1500 corresponde la zona 1. En X = -100 corresponde la zona -1.
`floor` es necesario para que las coordenadas negativas no se redondeen hacia cero.

Se recorren dos zonas hacia cada lado del centro: un cuadrado de 5 × 5, es
decir, 25 zonas. Eso permite generar objetos fuera de la vista antes de verlos.

Se rechazan coordenadas no finitas y zonas fuera del rango entero seguro antes
de convertir a `int`. El margen también evita desbordar los índices de los ciclos.

Estas zonas no son las baldosas del fondo: las baldosas actuales miden 1920 × 1080.

## 5. Generar una sola vez

```cpp
const auto clave = std::make_pair(x, y);
if (_zonas.find(clave) != _zonas.end())
{
    return;
}
```

`auto` deduce el tipo de la expresión. Aquí `clave` es una pareja de enteros.
`find` busca esa clave. `end()` representa que no se encontró un elemento.

Si la zona existe, la función termina antes de generar nuevamente. Así se
conservan posiciones y objetos usados al alejarse y volver.

La persistencia actual es en memoria durante la ejecución. No se escribe en
archivos `.dat` y cerrar el programa termina ese estado.

## 6. Semilla por zona y generador aleatorio

```cpp
std::seed_seq semillas{_semilla, static_cast<std::uint32_t>(x),
                      static_cast<std::uint32_t>(y)};
std::mt19937 generador(semillas);
```

`mt19937` es un generador de números pseudoaleatorios: con la misma semilla
reproduce la misma secuencia. `seed_seq` combina la semilla del mapa con las
coordenadas de esa zona.

`std::uint32_t` representa un entero sin signo de 32 bits. Las conversiones
permiten construir la secuencia con tipos compatibles. La identificación
de la zona en el mapa sigue usando sus coordenadas enteras con signo.

Con una misma semilla, explorar A antes que B da las mismas posiciones que
explorar B antes que A. El sorteo de recompensas del cofre utiliza el generador
de `Juego`, separado del generador local utilizado para construir cada zona.

## 7. Regiones separadas y lambda de posición

Fragmento de la generación:

```cpp
Zona zona{{
    {TipoEncuentro::Estacion, posicion(200.0f, 450.0f, 400.0f, 800.0f), false},
    {TipoEncuentro::Cofre, posicion(700.0f, 1000.0f, 200.0f, 400.0f), false},
    {TipoEncuentro::Cofre, posicion(700.0f, 1000.0f, 800.0f, 1000.0f), false}
}};
```

`posicion` es una lambda local: una función corta utilizada dentro de
`generarZona`. Recibe mínimos y máximos y sortea valores con
`std::uniform_real_distribution<float>`.

Su captura `[&]` permite consultar las variables locales necesarias por
referencia, incluido el generador. La lambda no se conserva después del método.

Al desplazamiento local se suma el origen de zona, `x * 1200` e `y * 1200`.
Por ejemplo, en zona X = 2, una posición local X = 300 produce X mundial = 2700.

Las regiones y los márgenes evitan amontonamientos. No se detectan estructuras
del PNG del suelo: este todavía no tiene un catálogo de objetos del entorno.

## 8. bernoulli_distribution y porcentajes actuales

```cpp
constexpr double PROBABILIDAD_COFRE = 0.25;
std::bernoulli_distribution sortearCofre(PROBABILIDAD_COFRE);
zona[1].presente = sortearCofre(generador);
zona[2].presente = sortearCofre(generador);
constexpr double PROBABILIDAD_ESTACION = 0.15;
zona[0].presente = std::bernoulli_distribution(PROBABILIDAD_ESTACION)(generador);
```

Una distribución de Bernoulli devuelve `true` con la probabilidad indicada
y `false` en los demás casos. Cada ubicación de cofre tiene 25%; la estación,
15% por zona. No se exige una cantidad exacta de apariciones en un grupo pequeño.

Con dos sorteos independientes de cofre por zona, las probabilidades son:

| Cofres en una zona | Probabilidad |
|---|---|
| Ninguno | 0,75 × 0,75 = 56,25% |
| Uno | 2 × 0,25 × 0,75 = 37,5% |
| Dos | 0,25 × 0,25 = 6,25% |

En promedio hay 0,5 cofres por zona, no dos garantizados. El sorteo de estación
se realiza después de los de cofres para no alterar su secuencia anterior.

## 9. Buscar el objeto dentro del radio de interacción

```cpp
if (!objeto.presente || objeto.usado || objeto.tipo != tipo)
{
    continue;
}
```

`continue` pasa al siguiente objeto. El filtro evita interactuar con ubicaciones
vacías, objetos consumidos y objetos de otro tipo.

```cpp
const float distancia = std::hypot(posicion.x - objeto.posicion.x,
                                   posicion.y - objeto.posicion.y);
```

`hypot` obtiene la distancia entre ambos puntos. Se conserva el objeto de menor
distancia dentro de 100 píxeles. `Juego` busca por tipo y compara estación y
cofre para resolver la tecla F. Los objetos ausentes tampoco se dibujan.

## 10. Compra de estación y puntero prestado

```cpp
if (jugador.gastarChatarra(_estacionMejoras.obtenerCosto()))
{
    _estacionSeleccionada = estacion;
    _estacionMejoras.abrirMenu();
}
```

Se cobra al abrir el menú. No se vuelve a cobrar mientras está abierto.
`_estacionSeleccionada` es un puntero al objeto del mapa: permite recordar
cuál estación debe marcarse usada después de elegir.

El puntero no es propietario: no corresponde hacer `delete` sobre él.
Insertar nuevas zonas en `std::map` conserva las referencias y punteros a
los objetos existentes.

Existe una sola instancia de `EstacionMejoras` en Juego: conserva los precios
y bonos generales. Las estaciones físicas son objetos del mapa con su propio
estado de uso, no instancias independientes del gestor de mejoras.

## 11. Opciones, pausa y consumo de la estación

Las tres opciones actuales son fijas:

| Opción | Efecto |
|---|---|
| Vida | +20 de vida máxima y actual. |
| Daño | +20% del daño base de ambas armas por aumento. |
| Cadencia | +15% de la velocidad base de ataque de ambas armas por aumento. |

La elección llega como índice 0, 1 o 2. `MejoraBasica& elegida` es una
referencia de salida: `confirmarEleccion` escribe la opción en la variable
que conserva el llamador. Un índice inválido no cierra ni consume la compra.

Después de confirmar:

```cpp
_estacionSeleccionada->usado = true;
_estacionSeleccionada = nullptr;
```

`->` accede a un miembro del objeto apuntado. `nullptr` indica que no queda
una estación pendiente. Cada estación permite una sola compra confirmada.

Mientras el menú está abierto, `Juego::actualizar` retorna antes de avanzar
la simulación. El reloj se reinicia para no acumular el tiempo de la pausa.

## 12. Cofre: pagar y recibir sin elegir

`intentarAbrirCofre` comprueba objeto cercano, jugador vivo, menú cerrado,
saldo y disponibilidad de recompensas. Si falla una condición, no cobra.

```cpp
if (!jugador.gastarChatarra(_costoCofre))
{
    return;
}
```

El descuento utiliza el método temporal autorizado de Jugador. La integración
deberá adaptarse al módulo definitivo del compañero; no se accede a su saldo
privado directamente.

Después de aplicar la recompensa se marca `cofre->usado = true`. No hay
menú de opciones y el resultado se muestra cinco segundos en pantalla.

## 13. Arma nueva y bonificaciones específicas

```cpp
const bool armaNueva = !_metralletaAdquirida &&
    ((!danoDisponible && !cadenciaDisponible) ||
     std::uniform_int_distribution<int>(0, 99)(_generador) < 15);
```

El sorteo entre 0 y 99 tiene 100 resultados posibles. Los valores de 0 a 14
son 15 resultados: 15% para metralleta mientras no esté adquirida.

Excepción: si ambos bonos del arma equipada se agotaron y falta la metralleta,
se entrega esa arma sin depender del sorteo. Una vez adquirida no vuelve a salir.
Adquirirla la equipa y habilita la selección con 2; inicialmente se posee pistola.

Los otros resultados dan daño o cadencia al arma equipada. Cada arma admite
cuatro aumentos de cada tipo desde cofres. Cuando solo queda un tipo de bono,
se entrega ese; si no queda ninguna recompensa, no se cobra.

```cpp
std::array<int, 2> _danoCofres{};
std::array<int, 2> _cadenciaCofres{};
```

Las llaves vacías inicializan los contadores en cero. Índice 0 representa
pistola y 1 metralleta. Los contadores locales se suman a los bonos generales
de la estación antes de llamar a `Arma::configurarBonificaciones`.

No modifican el nivel interno de Arma ni sus registros base. Son contadores
temporales de integración. Por ejemplo, dos aumentos de daño suman 40% del
daño base; no aplican dos multiplicaciones sucesivas de 1,2. El daño efectivo
se redondea hacia arriba: base 6 con un aumento de 20% da 8.

## 14. Precios independientes y aumento del 60%

| Tipo | Secuencia inicial |
|---|---|
| Estaciones | 10 → 16 → 26 → 42 → 68 |
| Cofres | 15 → 24 → 39 → 63 → 101 |

Las estaciones guardan su precio en `EstacionMejoras`; el cofre usa
`_costoCofre` de Juego. Son precios globales por categoría: cada cofre cerrado
muestra el costo actual de la próxima apertura, no un precio individual fijo.

```cpp
const long long siguienteCosto =
    (static_cast<long long>(_costoCofre) * 8 + 4) / 5;
```

Multiplicar por 8 y dividir por 5 equivale a multiplicar por 1,6. Como la
división es entera, sumar 4 permite redondear hacia arriba un costo positivo.

```text
Costo 24: (24 × 8 + 4) / 5 = 196 / 5 = 39
Costo 39: (39 × 8 + 4) / 5 = 316 / 5 = 63
```

`static_cast<long long>` convierte antes de multiplicar. Declarar solamente
el resultado como `long long` no impediría que una multiplicación previa de
enteros `int` se desbordara.

```cpp
_costoCofre = static_cast<int>(std::min(siguienteCosto,
    static_cast<long long>(std::numeric_limits<int>::max())));
```

Se limita al mayor valor representable por `int` antes de volver a convertir.
El precio sube después de una recompensa efectiva; en estaciones, al confirmar.

## 15. Reinicio y devolución del pago pendiente

```cpp
if (_estacionMejoras.menuAbierto())
{
    jugador.ganarChatarra(_estacionMejoras.obtenerCosto());
    _estacionMejoras.cancelarCompra();
}
```

El precio todavía no aumentó mientras se esperaba la elección, por eso se
devuelve ese valor. Se libera la referencia pendiente sin marcar la estación usada.

R conserva zonas, objetos usados, precios, bonos y armas adquiridas. Reinicia
la prueba de combate y vuelve a seleccionar la pistola. No equivale a comenzar
una partida completamente nueva ni a recuperar el estado desde disco.

## 16. Dibujar estado y corregir posición

Cada objeto se dibuja con su coordenada del mapa:

```cpp
const Posicion posicion = objeto.posicion;
```

Usar la posición del gestor global de estaciones causó que su dibujo no
coincidiera con el punto de interacción. Se corrigió utilizando esta posición.

Una estación usada queda con pantalla apagada. Un cofre usado se dibuja oscuro
y con tapa abierta. Permanecen visibles, pero la búsqueda los descarta.

Solo se dibujan objetos próximos a la vista. El mapa conserva los demás para
recordar su estado al regresar. Los dibujos siguen siendo figuras provisionales.
