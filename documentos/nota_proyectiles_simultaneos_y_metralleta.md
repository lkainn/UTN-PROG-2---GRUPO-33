# Nota: proyectiles simultáneos y segunda arma


Esta nota explica la implementación actual del combate para poder estudiarla,
seguir sus métodos y defender las decisiones del código. Los fragmentos muestran
el código relevante; cuando se omiten partes, se indica expresamente.

## 1. Qué cambió y por qué

Antes, `Juego` conservaba un único `Proyectil`. Aunque el arma hubiera terminado
su tiempo entre ataques, debía esperar a que ese proyectil impactara o saliera
de la ventana. La distancia al enemigo terminaba condicionando la frecuencia
real de disparo.

Ahora cada disparo tiene su propio objeto. `Juego` conserva los proyectiles en
un vector, actualiza sus movimientos, resuelve sus impactos y elimina los que
terminaron. El arma puede crear otro disparo cuando finaliza su enfriamiento.

Sobre esa base se agregó una metralleta: dispara con más frecuencia, pero cada
proyectil hace menos daño. Es otra instancia de `Arma`, sin una clase nueva ni
herencia. Las teclas `1` y `2` sirven exclusivamente para probar las dos armas.
La obtención definitiva mediante progreso o mejoras está pendiente de acordar.

## 2. Archivos que conviene abrir

| Archivo | Qué estudiar |
| --- | --- |
| `include/Juego.h` | Vector, armas, selección y declaraciones de métodos. |
| `src/Juego.cpp` | Creación, movimiento, colisiones, limpieza, dibujo y reinicio. |
| `include/Arma.h` y `src/Arma.cpp` | Estadísticas, referencias y tiempo entre ataques. |
| `include/Proyectil.h` y `src/Proyectil.cpp` | Estado independiente de cada disparo. |
| `src/SistemaColisiones.cpp` | Intersección de rectángulos y límites. |
| `pruebas/JuegoProyectiles.cpp` | Comprobaciones del ciclo real de Juego. |
| `pruebas/probar_juego_proyectiles.bat` | Compilación y ejecución de esas comprobaciones. |

La implementación nueva se concentra en `Juego`. Se reutilizó el código de
`Arma`, `Proyectil`, `Jugador` y `Enemigo` sin modificar esas clases para estos
dos incrementos.

## 3. Responsabilidades de cada clase

| Clase | Responsabilidad |
| --- | --- |
| `Arma` | Guardar estadísticas, controlar la cadencia y configurar un disparo válido. |
| `Proyectil` | Guardar posición, velocidad, tamaño, daño y estado; mover el disparo. |
| `SistemaColisiones` | Informar si dos rectángulos intersectan o si uno está dentro de otro. |
| `Juego` | Elegir objetivo y arma; almacenar disparos; aplicar impactos; administrar recompensas y dibujo. |

El arma no recorre enemigos ni dibuja. El proyectil no decide qué enemigo recibe
daño. El sistema de colisiones no entrega recompensas. Esa coordinación ocurre
en `Juego`.

## 4. El vector de proyectiles

```cpp
std::vector<Proyectil> _proyectiles;
```

`std::vector` permite que la cantidad de elementos cambie durante la ejecución.
Aquí almacena objetos `Proyectil` por valor: cada elemento tiene sus propios
atributos y puede moverse o desactivarse de manera independiente.

Ejemplo conceptual:

```text
_proyectiles
    [0] → posición (400, 200), daño 25, activo
    [1] → posición (250, 200), daño 6, activo
    [2] → posición (150, 200), daño 6, activo
```

Mover el elemento `[0]` no modifica los otros. El vector puede estar vacío si
no se disparó, si todos los disparos terminaron o si se reinició la prueba.

### Memoria y propiedad

`Juego` es propietario de la colección. El vector administra automáticamente
su almacenamiento, por lo que no necesitamos `new`, `delete` ni punteros para
cada disparo. Esto aplica RAII: la liberación se vincula a la vida del objeto
que administra el recurso.

Al crecer, el vector puede reservar un bloque mayor y trasladar sus elementos.
Eso puede invalidar referencias o iteradores anteriores. Por esa razón no
guardamos referencias persistentes a proyectiles ni agregamos elementos durante
el recorrido que los actualiza.

`clear()` destruye todos los elementos, pero puede conservar capacidad para
reutilizarla. No es una fuga: el almacenamiento se libera automáticamente al
destruir el vector.

## 5. Estadísticas de las armas

En la lista de inicialización del constructor de `Juego` aparecen:

```cpp
armaPrueba(1, "Pistola basica", 25, 2.0f, ALCANCE_ARMA, 5),
_metralletaPrueba(2, "Metralleta", 6, 8.0f, ALCANCE_ARMA, 5),
_armaSeleccionada(TipoArmaPrueba::Pistola),
```

El orden de parámetros de `Arma` es: identificador, nombre, daño, ataques por
segundo, alcance y nivel máximo.

| Característica | Pistola | Metralleta |
| --- | ---: | ---: |
| Identificador | 1 | 2 |
| Daño por disparo | 25 | 6 |
| Ataques por segundo configurados | 2 | 8 |
| Intervalo nominal | 0,5 s | 0,125 s |
| Daño por segundo teórico | 50 | 48 |
| Alcance temporal | 2210 px | 2210 px |
| Velocidad del proyectil | 800 px/s | 800 px/s |
| Tamaño del proyectil | 28 × 14 px | 14 × 6 px |
| Color | Amarillo | Naranja |
| Nivel máximo registrado | 5 | 5 |

El daño por segundo teórico se calcula multiplicando daño por frecuencia:
`25 × 2 = 50` y `6 × 8 = 48`. Supone disparos continuos que impactan. No mide
la eficacia real frente a enemigos que se mueven.

La velocidad y las dimensiones del proyectil se eligen en `Juego`; no son
atributos almacenados en `Arma`. El nivel máximo existe en el objeto, pero el
nivel del arma todavía no modifica sus estadísticas.

## 6. Selección temporal del arma

```cpp
enum class TipoArmaPrueba
{
    Pistola,
    Metralleta
};
```

El `enum class` representa las dos opciones por nombre. Se escribe
`TipoArmaPrueba::Metralleta` para indicar a qué enumeración pertenece el valor.

La selección solo modifica una variable:

```cpp
void Juego::seleccionarArmaPrueba(TipoArmaPrueba tipo)
{
    _armaSeleccionada = tipo;
}
```

No crea otra arma, no reinicia enfriamientos y no borra proyectiles. En
`procesarEventos()`, las teclas `Num1` y `Numpad1` seleccionan la pistola;
`Num2` y `Numpad2` seleccionan la metralleta.

Este control está identificado como prueba en el título de la ventana. No
representa que el jugador ya haya obtenido ambas armas dentro de una partida
definitiva. Esa regla se implementará cuando se acuerde el sistema de progreso.

### Obtener el objeto elegido por referencia

```cpp
Arma& Juego::obtenerArmaSeleccionada()
{
    if (_armaSeleccionada == TipoArmaPrueba::Metralleta)
    {
        return _metralletaPrueba;
    }
    return armaPrueba;
}
```

El retorno `Arma&` es una referencia al objeto que ya pertenece a `Juego`.
Después se utiliza así:

```cpp
Arma& arma = obtenerArmaSeleccionada();
```

Cuando se modifica el enfriamiento de `arma`, se modifica el de la pistola o
metralleta real. Si se obtuviera una copia, se actualizaría otro objeto y el
enfriamiento original no recibiría ese cambio.

## 7. Cadencia y enfriamientos independientes

Un disparo exitoso asigna en `Arma`:

```cpp
_tiempoRestante = 1.0f / _velocidadAtaque;
```

Para ocho ataques por segundo, el intervalo es `1 / 8 = 0,125` segundos.
Mientras el contador sea positivo, `puedeDisparar()` devuelve falso.

Cada actualización del arma descuenta el tiempo:

```cpp
_tiempoRestante = std::max(0.0f, _tiempoRestante - segundos);
```

`std::max` impide que quede un valor negativo. `Arma::actualizar()` también
rechaza tiempos no finitos o menores o iguales a cero.

`Juego` actualiza ambas armas, incluso la que no está seleccionada:

```cpp
void Juego::actualizarArmas(float segundos)
{
    armaPrueba.actualizar(segundos);
    _metralletaPrueba.actualizar(segundos);
}
```

Ejemplo: la pistola dispara y queda con 0,5 segundos de espera. Seleccionamos la
metralleta y pasan 0,125 segundos. La pistola habrá reducido su espera a 0,375
segundos. Volver a seleccionarla no la habilita antes de tiempo.

Cada arma comienza disponible, por lo que se puede hacer un primer disparo de
pistola y luego un primer disparo de metralleta al alternar. No existe un
enfriamiento global compartido: la garantía actual es que cambiar no reinicia
el contador de cada arma.

Los intervalos son nominales. La comprobación se realiza por fotograma y se
crea como máximo un disparo en cada actualización. Si el contador llega a cero
entre dos fotogramas, se dispara en la siguiente comprobación. No se recuperan
disparos acumulados tras una pausa; no hay una temporización exacta de ocho
disparos en toda ventana de un segundo.

## 8. Elegir objetivo y crear un disparo

`actualizarDisparoAutomatico()` comienza con:

```cpp
Arma& arma = obtenerArmaSeleccionada();
if (!jugador.estaVivo() || !arma.puedeDisparar())
{
    return;
}
```

Ya no pregunta si existe otro disparo activo. Esa era la condición que
limitaba el combate a un proyectil en vuelo.

Después calcula el centro del jugador y recorre enemigos vivos para elegir
el centro más cercano. Compara distancias al cuadrado:

```cpp
const float distanciaCuadrada =
    diferenciaX * diferenciaX + diferenciaY * diferenciaY;
```

Para elegir la menor distancia no hace falta calcular una raíz cuadrada en
cada comparación. Si no hay enemigo vivo, el método termina sin disparar.

### El objeto inicialmente inactivo

```cpp
Proyectil nuevoProyectil({0.0f, 0.0f}, 0.0f, 0.0f, 0.0f, 0.0f, 0);
```

Sus dimensiones son cero y el constructor exige ancho y alto positivos para
activarlo. Se usa como destino de la configuración que realizará el arma.

`Arma::intentarDisparar()` conserva esta validación:

```cpp
if (!puedeDisparar() || proyectil.estaActivo() ||
    !std::isfinite(velocidadProyectil) || velocidadProyectil <= 0.0f)
{
    return false;
}
```

Esto evita sobrescribir un objeto que ya representa un disparo activo. La
restricción se aplica al objeto recibido, no a los demás elementos del vector.

### Seleccionar el tamaño sin depender de la figura reutilizada

```cpp
const bool esMetralleta = _armaSeleccionada == TipoArmaPrueba::Metralleta;
const float ancho = esMetralleta ? ANCHO_PROYECTIL_METRALLETA : ANCHO_PROYECTIL_PISTOLA;
const float alto = esMetralleta ? ALTO_PROYECTIL_METRALLETA : ALTO_PROYECTIL_PISTOLA;
```

El operador ternario `condicion ? valorSiVerdadero : valorSiFalso` elige las
dimensiones. Se usan constantes porque la figura gráfica cambia de tamaño al
dibujar cada disparo. Leer su último tamaño para crear un proyectil podría
asignar dimensiones del arma equivocada.

Finalmente:

```cpp
if (arma.intentarDisparar(
        nuevoProyectil, centroJugador, centroObjetivo, VELOCIDAD_PROYECTIL,
        ancho, alto))
{
    _proyectiles.push_back(nuevoProyectil);
}
```

El arma recibe `nuevoProyectil` por referencia, calcula dirección, posición
inicial y daño, y lo configura. Solo si devuelve `true` se agrega al vector.
Un objetivo fuera de alcance o coincidente con el origen no consume enfriamiento.

`push_back` guarda una copia del objeto. Al salir del método se destruye el
objeto local, pero el elemento del vector continúa existiendo.

## 9. Dirección y posición inicial

Estas operaciones ya existían en `Arma` y se reutilizan para ambas armas:

```cpp
const float distancia = std::hypot(diferenciaX, diferenciaY);
const float velocidadX = diferenciaX / distancia * velocidadProyectil;
const float velocidadY = diferenciaY / distancia * velocidadProyectil;
```

Dividir las diferencias por la distancia normaliza la dirección. Así un
disparo diagonal no resulta más rápido que uno horizontal.

Ejemplo con diferencias `(300, 400)` y velocidad 800:

```text
distancia = 500
velocidadX = 300 / 500 × 800 = 480
velocidadY = 400 / 500 × 800 = 640
magnitud de la velocidad = 800
```

El origen recibido representa el centro del jugador. La posición guardada en
el proyectil representa la esquina superior izquierda de su rectángulo:

```cpp
const Posicion posicionInicial{
    origen.x - anchoProyectil / 2.0f,
    origen.y - altoProyectil / 2.0f
};
```

Si el centro es `(100, 100)` y el tamaño de metralleta es `(14, 6)`, la esquina
inicial queda en `(93, 97)`. El centro del disparo coincide con el del jugador.

## 10. Orden dentro del fotograma

La actualización sigue este orden relevante, omitiendo otras operaciones:

```cpp
actualizarArmas(segundos);
// Entrada, jugador, enemigos e indicadores.
actualizarProyectiles(segundos);
actualizarRecursos(segundos);
actualizarDisparoAutomatico();
```

Se procesan los disparos existentes antes de agregar uno nuevo. Por eso el
recién creado puede dibujarse en su origen durante ese fotograma y comienza
a moverse en el siguiente. Tampoco se modifica el tamaño del vector mientras
el recorrido de actualización mantiene referencias a sus elementos.

El tiempo del fotograma se limita a 0,05 segundos en `Juego`. Esto reduce los
saltos de movimiento tras pausas, aunque no equivale a detectar todas las
colisiones a lo largo de una trayectoria.

## 11. Movimiento de todos los disparos

Fragmento de `actualizarProyectiles()`:

```cpp
for (Proyectil& proyectil : _proyectiles)
{
    if (!proyectil.estaActivo())
    {
        continue;
    }

    proyectil.actualizar(segundos);
    // Comprobación de impactos y límites.
}
```

`Proyectil&` permite modificar el elemento real del vector. Sin `&`, se
actualizaría una copia y el elemento original no conservaría el movimiento.
`continue` salta la vuelta de un disparo que ya está inactivo.

Cada objeto aplica:

```cpp
_posicion.x += _velocidadX * segundos;
_posicion.y += _velocidadY * segundos;
```

Con posición x = 100, velocidad horizontal = 800 y tiempo = 0,1 segundos,
la nueva x es `100 + 800 × 0,1 = 180`.

## 12. Un impacto por proyectil

Dentro del recorrido de disparos se recorren los enemigos:

```cpp
for (std::size_t i = 0; i < CANTIDAD_ENEMIGOS; i++)
{
    if (enemigosPrueba[i].estaVivo() &&
        SistemaColisiones::hayInterseccion(
            proyectil.obtenerFormaColision(),
            enemigosPrueba[i].obtenerFormaColision()))
    {
        enemigosPrueba[i].recibirDanio(proyectil.obtenerDano());
        proyectil.desactivar();
        if (!enemigosPrueba[i].estaVivo())
        {
            generarRecompensa(enemigosPrueba[i]);
            tiemposReaparicion[i] = SEGUNDOS_REAPARICION;
        }
        break;
    }
}
```

El daño se consulta en el proyectil, no en el arma seleccionada. Por eso un
disparo de pistola sigue haciendo 25 aunque se cambie a metralleta antes del
impacto, y uno de metralleta conserva su daño de 6.

`break` termina únicamente el recorrido interno de enemigos. El recorrido
externo continúa con el siguiente proyectil. Así un disparo no puede dañar
a dos enemigos en el mismo fotograma.

El primer enemigo encontrado es el primero según el orden del arreglo entre
los que intersectan la posición actual; no se calcula el primer obstáculo
a lo largo del segmento recorrido. El proyectil tampoco queda vinculado al
objetivo inicial: puede impactar a otro enemigo que se cruce.

### Por qué no se duplica la recompensa

La recompensa se genera cuando un enemigo que estaba vivo queda muerto después
del impacto. Los siguientes disparos omiten ese enemigo porque `estaVivo()`
ya devuelve falso. Además, el disparo que impactó queda desactivado y se elimina.

Ejemplo: un enemigo tiene 7 de vida. Un disparo de metralleta hace 6 y lo deja
con 1. El segundo lo mata y genera una recompensa de experiencia y otra de
chatarra, si ambas cantidades son positivas. Un tercer disparo no vuelve a
premiar esa muerte.

## 13. Salida de pantalla y desactivación

```cpp
if (proyectil.estaActivo() &&
    !SistemaColisiones::estaDentroDe(
        proyectil.obtenerFormaColision(), escenario))
{
    proyectil.desactivar();
}
```

`escenario` es el rectángulo de la ventana. Se exige que el rectángulo completo
del disparo esté dentro: si una parte sobresale, se desactiva. La condición
de activo evita volver a procesar por límites un disparo que ya impactó.

Desactivar significa cambiar `_activo` a `false`. Todavía no elimina el objeto
del vector; la eliminación se hace después de recorrer todos los disparos.

## 14. Limpieza con remove_if y erase

```cpp
_proyectiles.erase(
    std::remove_if(
        _proyectiles.begin(), _proyectiles.end(),
        [](const Proyectil& proyectil)
        {
            return !proyectil.estaActivo();
        }),
    _proyectiles.end());
```

La expresión `[](const Proyectil& proyectil) { ... }` es una lambda: una función
pequeña que en este caso indica qué elementos deben retirarse. Devuelve `true`
para un proyectil inactivo. Los corchetes vacíos indican que no captura
variables externas.

`begin()` señala el inicio del rango y `end()` su límite final. `remove_if`
reorganiza el rango para dejar los elementos conservados al comienzo y devuelve
el inicio de la parte sobrante. No reduce el tamaño del vector por sí solo.
`erase` elimina esa parte y reduce su cantidad de elementos.

```text
Antes:   [A activo] [B inactivo] [C activo] [D inactivo]
Después: [A activo] [C activo]
```

La parte sobrante intermedia no debe interpretarse como una lista confiable
de los objetos retirados: la operación puede haber movido elementos.

Se limpia después del recorrido para no invalidar las referencias que ese
recorrido utiliza. No se ejecuta `delete` porque se almacenan objetos por valor.

## 15. Dibujo y apariencia al cambiar de arma

Se reutiliza un único `sf::RectangleShape`, pero se lo configura antes de
dibujar cada disparo:

```cpp
const FormaColision forma = proyectil.obtenerFormaColision();
figuraProyectilPrueba.setSize({forma.ancho, forma.alto});
figuraProyectilPrueba.setPosition({forma.x, forma.y});
const bool esMetralleta =
    forma.ancho == ANCHO_PROYECTIL_METRALLETA &&
    forma.alto == ALTO_PROYECTIL_METRALLETA;
figuraProyectilPrueba.setFillColor(
    esMetralleta ? COLOR_PROYECTIL_METRALLETA : COLOR_PROYECTIL_PISTOLA);
ventana.draw(figuraProyectilPrueba);
```

En ese recorrido se usa `const Proyectil&` porque solo se consultan datos.
La figura cambia; el estado del proyectil no se modifica para dibujarlo.

Se dibuja A en su posición y luego B en la suya. Cambiar la figura para B no
borra A de ese fotograma. `ventana.display()` presenta el resultado después.

El tamaño se obtiene del disparo y el color se deduce de su formato. Por eso
cambiar de arma no transforma visualmente los proyectiles anteriores.

Esta deducción del color es una decisión temporal para dos formatos conocidos.
Si otra arma usa también 14 × 6, aparecería naranja; si se modifican tamaños,
hay que revisar la regla. La vista no almacena un identificador de arma por
disparo. Ese diseño podrá revisarse cuando haya más variantes.

## 16. Nombre en la interfaz y reinicio

La interfaz construye el nombre a partir del arma elegida:

```cpp
sf::Text nombreArma(
    fuenteDanio,
    "ARMA: " + std::string(obtenerArmaSeleccionada().obtenerNombre()), 22);
```



Al presionar `R`, estas instrucciones forman parte del reinicio:

```cpp
_proyectiles.clear();
armaPrueba.reiniciar();
_metralletaPrueba.reiniciar();
seleccionarArmaPrueba(TipoArmaPrueba::Pistola);
```

Se eliminan los disparos, ambos enfriamientos quedan en cero y se selecciona
la pistola. El método también restaura jugador y enemigos, elimina recursos
e indicadores y reinicia temporizadores de la demostración.
