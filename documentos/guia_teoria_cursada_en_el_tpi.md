# La teoría de la cursada dentro del TPI

El profesor va a preguntar teoría de Programación 2 **y** dónde se aplica en el
proyecto. Esta guía cruza las dos cosas: para cada tema, qué dice la teoría, un
ejemplo bobo para explicarlo con las propias palabras, dónde está en el código,
y la respuesta a la pregunta probable.

Hay tres temas que el proyecto **no usa**, y son los más peligrosos porque
parecen un olvido. No lo son:

| Tema | ¿Está en el TPI? |
|---|---|
| Herencia | **no**, y hay un motivo |
| `new` / `delete` | **no**, los `vector` la administran |
| Destructores propios | **no**, no hacen falta |
| Puntero `this` | **no explícitamente**, pero está en cada método |

---

# Parte 1 — Las diferencias que más se preguntan

Casi toda pregunta teórica es en el fondo *«¿cuál es la diferencia entre A y
B?»*. Acá están las catorce que pueden salir, con el ejemplo para explicarlas.

## Clase y objeto

| | Clase | Objeto |
|---|---|---|
| Qué es | el molde | cada cosa hecha con ese molde |
| Cuántos hay | uno | todos los que hagan falta |
| Cuándo existe | lo escribís vos, en el `.h` | se crea mientras el juego corre |
| En el TPI | `class Enemigo` | cada enemigo que te persigue |

> **Ejemplo bobo:** la clase es la **budinera**. El objeto es cada **budín** que
> sacás del horno. Hay una sola budinera y cien budines, todos con la misma
> forma pero cada uno con su propio relleno.

**En el proyecto hay tres niveles, y conviene no mezclarlos:**

```
class Enemigo            ← la budinera: define qué tiene un enemigo
fila de Enemigos.dat     ← la receta: "robot rápido, 60 de vida, 220 px/s"
el enemigo en pantalla   ← el budín: uno concreto, en (340, 820), con 45 de vida
```

Por eso el archivo guarda **cuatro filas** y en pantalla puede haber **sesenta
enemigos**: cuatro recetas, sesenta budines.

## Atributo y método

| | Atributo | Método |
|---|---|---|
| Qué es | un dato que el objeto guarda | algo que el objeto sabe hacer |
| Ejemplo | `int vida;` | `void recibirDanio(int);` |

> **Ejemplo bobo:** en una heladera, los **atributos** son la temperatura y
> cuánta comida tiene adentro; los **métodos** son enfriar y hacer hielo.

## `public` y `private`

| | `private` | `public` |
|---|---|---|
| Quién lo toca | solo la propia clase | cualquiera |
| Qué va ahí | los datos | lo que la clase ofrece hacer |

> **Ejemplo bobo:** el **cajero automático**. La plata y el mecanismo están
> adentro (`private`); los botones están afuera (`public`). Podés pedir «dame
> $1000», pero no meter la mano y agarrar lo que quieras.

## Declaración y definición (`.h` y `.cpp`)

| | `.h` — declaración | `.cpp` — definición |
|---|---|---|
| Qué dice | **qué** sabe hacer la clase | **cómo** lo hace |
| Quién lo lee | todos los que usen la clase | solo el compilador |

> **Ejemplo bobo:** el `.h` es el **menú del restaurante** —dice qué platos
> hay—; el `.cpp` es la **cocina**, donde están las recetas. El cliente lee el
> menú; no necesita entrar a la cocina.

## Constructor y destructor

| | Constructor | Destructor |
|---|---|---|
| Cuándo corre | al crear el objeto | al destruirlo |
| Para qué | dejar los datos en un estado válido | devolver lo que haya pedido prestado |
| Se llama | igual que la clase | igual, con `~` adelante |
| En el TPI | en todas las clases | **en ninguna** |

> **Ejemplo bobo:** entrar y salir de un **hotel**. El constructor es el
> *check-in*: te anotan y te dan la llave, no podés estar en el hotel sin pasar
> por ahí. El destructor es el *check-out*: devolvés la llave. Si nunca pediste
> llave, no hay nada que devolver — por eso nuestras clases no tienen
> destructor.

## El puntero `this` y el guion bajo

Las dos formas resuelven el mismo problema: distinguir el parámetro del
atributo cuando se llaman igual.

```cpp
Enemigo::Enemigo(int vida)  { this->vida = vida; }   // con this
Enemigo::Enemigo(int _vida) { vida = _vida; }        // lo que hicimos
```

> **Ejemplo bobo:** en una familia donde el padre y el hijo se llaman Juan, hay
> que decir «Juan **padre**» o «Juan **hijo**». `this->` es decir «el de la
> casa»; el guion bajo es ponerle apodo al visitante.

**Si no hacés ninguna de las dos**, `vida = vida;` se asigna el parámetro a sí
mismo, el atributo nunca se carga, **y compila sin avisar nada**.

## `enum` y `enum class`

| | `enum` clásico | `enum class` |
|---|---|---|
| Cómo se escribe | `BASICO` | `ResultadoPartida::Victoria` |
| ¿Convierte a `int` solo? | **sí** | **no**, hay que pedirlo |
| Si dos listas repiten un nombre | choca | no pasa nada |
| En el TPI | `TipoEnemigo` | los otros seis |

> **Ejemplo bobo:** «rojo» puede ser un **color de pintura** o una **luz del
> semáforo**. Si decís solo «rojo» en una ferretería que además vende
> semáforos, nadie sabe cuál. `enum class` te obliga a decir
> «Semáforo::rojo» — el apellido es obligatorio.

## Herencia y composición

| | Herencia | Composición |
|---|---|---|
| La relación | **«es un»** | **«tiene un»** |
| Ejemplo | un perro **es un** animal | un auto **tiene un** motor |
| Se escribe | `class Perro : public Animal` | `class Auto { Motor motor; };` |
| En el TPI | **no se usa** | en todos lados |

> **Ejemplo bobo:** herencia es el **apellido**: heredás de tu familia y sos de
> esa familia. Composición es **lo que tenés en la mochila**: no *sos* un
> cuaderno, pero *tenés* uno.

## Pila y montículo (dónde vive la memoria)

| | Pila (*stack*) | Montículo (*heap*) |
|---|---|---|
| Cuándo se decide el tamaño | al compilar | mientras el programa corre |
| Quién la libera | sola, al salir de la función | vos, o un `vector` por vos |
| Ejemplo | `int vida;` `char nombre[30];` | lo que guarda un `std::vector` |

> **Ejemplo bobo:** la pila es el **escritorio**: chico, ordenado, y cuando
> terminás el día se limpia solo. El montículo es el **depósito**: entra todo
> lo que quieras, pero si no avisás que sacaste algo, queda ocupado para
> siempre.

## Arreglo fijo y `std::vector`

| | `char nombre[30]` | `std::vector<Enemigo>` |
|---|---|---|
| Tamaño | fijo, decidido al escribir el código | crece y se achica solo |
| Dónde vive | con el objeto | en el montículo |
| Sirve para archivos | **sí** | no directamente |

> **Ejemplo bobo:** el arreglo es la **maple de 12 huevos**: entran 12, ni uno
> más. El vector es la **bolsa de tela**: metés lo que quieras y se estira.

## `char[30]` y `std::string`

> **Ejemplo bobo:** `char[30]` es la **etiqueta pegada** en el frasco: el texto
> está ahí, escrito encima. `std::string` es un **papelito que dice «el texto
> está en el cajón 4»**. Si fotocopiás el frasco te llevás la etiqueta; si
> fotocopiás el papelito, te llevás una dirección que en otra casa no existe.

Por eso los archivos binarios usan `char[30]`: `fwrite` saca una fotocopia de
los bytes, y la dirección de memoria no sirve en la próxima ejecución.

## Archivo de texto y archivo binario

| | Texto | Binario |
|---|---|---|
| Qué guarda | caracteres legibles | los bytes tal como están en memoria |
| Se puede abrir con | el Bloc de notas | no, se ve como basura |
| Cuánto ocupa el número 1000000 | 7 caracteres | 4 bytes (un `int`) |

> **Ejemplo bobo:** texto es **escribir la receta a mano**; binario es
> **fotocopiar la página**. La fotocopia es más rápida y exacta, pero solo la
> entiende quien tenga el mismo libro.

## `"wb"` y `"ab"`

| | `"wb"` | `"ab"` |
|---|---|---|
| Qué hace | vacía el archivo y escribe de cero | agrega al final |
| En el TPI | los catálogos | `Partidas.dat` |

> **Ejemplo bobo:** `"wb"` es la **pizarra**: borrás todo y escribís de nuevo.
> `"ab"` es el **cuaderno**: agregás una hoja y lo anterior queda.

## Por valor y por referencia

```cpp
void f(Enemigo e);          // por valor: recibe una copia
void f(Enemigo& e);         // por referencia: trabaja sobre el original
void f(const Enemigo& e);   // por referencia, pero sin poder modificarlo
```

> **Ejemplo bobo:** por valor es darle una **fotocopia** del documento: lo puede
> rayar todo, el original no se entera. Por referencia es prestarle **el
> original**. Y `const` es prestarle el original **con un cartel de «no
> tocar»**.

En el TPI esto aparece en `for (Enemigo& e : enemigos)`: **sin el `&` se
mueve la fotocopia y el enemigo real queda quieto**, sin ningún error.

## `static` en un método y `static` en un atributo

| | Método `static` | Atributo `static` |
|---|---|---|
| Qué significa | no necesita un objeto para llamarse | hay **uno solo** para toda la clase |
| En el TPI | `SistemaColisiones::hayInterseccion()` | el contador de `idInstancia` |

> **Ejemplo bobo:** el método `static` es la **calculadora del mostrador**:
> cualquiera la usa, no es de nadie. El atributo `static` es el **rollito de
> números** del Registro Civil: uno solo para todos, y cada persona que llega
> arranca el suyo.

---

# Parte 2 — Tema por tema

## 1. POO: clase, objeto, encapsulamiento

**La teoría.** Una clase es el molde; un objeto es cada ejemplar. El
encapsulamiento consiste en declarar los atributos `private` y dar acceso
controlado con métodos `public`, para que nadie pueda dejar el objeto en un
estado inválido.

**En el TPI.** Ninguna clase tiene atributos públicos. El caso más claro:

```cpp
class Enemigo {
    private:
        int vida;              // nadie puede escribirlo desde afuera
    public:
        void recibirDanio(int cantidad);   // la única forma de bajarlo
        bool estaVivo() const;
};
```

**La pregunta probable:** *«¿por qué no hacés pública la vida y listo?»*

Porque entonces cualquiera podría escribir `enemigo.vida = 9999` y saltearse las
reglas. Con `recibirDanio()`, la lógica de cómo cambia la vida —incluido el tope
en cero— vive adentro de la clase y vale para todos los que la usen.

**Un matiz que suma:** no todos los atributos tienen *setter*. `Enemigo` no
tiene `setVida()`, solo `recibirDanio()`. Poner un *setter* por cada atributo
equivale a dejarlos públicos con pasos de más; los métodos deben representar lo
que la entidad **hace**.

---

## 2. Constructores y destructores

**La teoría.** El constructor se ejecuta solo al crear el objeto y deja sus
atributos en un estado válido. El destructor (`~Clase()`) se ejecuta al
destruirlo y libera lo que el objeto haya reservado.

**Por qué hace falta el constructor.** Sin él, un objeto recién creado tiene
basura de memoria. El apunte de la cátedra lo muestra con una salida real del
programa: números como `1161838592` y caracteres ilegibles.

> **Ejemplo bobo:** es como recibir un formulario **sin completar**, con lo que
> el anterior escribió a medias todavía encima. El constructor es la regla de
> «antes de usarlo, se completa».

**En el TPI.** Dos formas conviven:

```cpp
// Enemigo: asignación dentro del cuerpo
Enemigo::Enemigo(int _idEnemigo, float _x, ...) {
    idEnemigo = _idEnemigo;
    x = _x;
}

// Arma: lista de inicialización, antes de entrar al cuerpo
Arma::Arma(int id, const char* nombre, ...)
    : _id(id > 0 ? id : 0),
      _dano(dano) {
}
```

**Destructores: el proyecto no define ninguno**, y está bien.

**La pregunta probable:** *«¿por qué ninguna clase tiene destructor?»*

Porque ninguna reserva recursos que haya que liberar a mano. Los `vector` sueltan
su memoria solos, y los `FILE*` se cierran con `fclose()` en la misma función que
los abrió. Un destructor vacío no agregaría nada. Si alguna clase pasara a usar
`new`, ahí sí haría falta.

---

## 3. El puntero `this`

**La teoría.** De los apuntes: *«`this` es un puntero que contiene la dirección
del objeto que llama a un método»*. Existe porque **los datos se guardan una vez
por objeto, pero el código del método es uno solo y compartido**.

> **Ejemplo bobo:** una **fotocopiadora** en una oficina de cien personas. La
> máquina es una sola y la usan todos, pero cada uno copia *su* carpeta.
> `this` es la carpeta que tenés apoyada en ese momento: lo que le dice a la
> máquina sobre cuál de los cien trabajos está operando.

**En el TPI: no aparece escrito en ninguna parte.** Está implícito en cada
método —cada vez que `recibirDanio()` toca `vida`, está tocando la del objeto
que lo llamó.

**La pregunta probable:** *«¿usaron `this`? ¿dónde lo necesitarían?»*

No explícitamente, porque se evitó el caso que lo hace necesario: cuando un
parámetro se llama igual que un atributo. Ver la comparación de la Parte 1.

---

## 4. Enumeradores

**La teoría.** Un `enum` da nombre a un conjunto cerrado de valores, en lugar de
usar enteros sueltos sin significado. El clásico expone sus nombres en el ámbito
donde se declara y convierte a `int` solo; el `enum class` de C++11 encierra cada
valor en su tipo, obliga a escribir `Tipo::VALOR`, y **no** convierte a `int` sin
`static_cast`.

> **Ejemplo bobo:** sin `enum`, el tipo de enemigo sería `1`, `2`, `3`. Si ves
> `crearEnemigo(3, ...)` no sabés qué es. Con `enum`, ves
> `crearEnemigo(RESISTENTE, ...)`. Y si escribís `RESITENTE`, **no compila** —
> con un número, `crearEnemigo(7, ...)` compila igual y falla jugando.

**En el TPI hay siete, de los dos tipos:**

| Enumerador | Forma | Dónde |
|---|---|---|
| `TipoEnemigo` | `enum` clásico | `Enemigo.h` |
| `ResultadoPartida` | `enum class` | `Partida.h` |
| `RarezaMejora` | `enum class` | `InventarioArmas.h` |
| `TipoEncuentro` | `enum class` | `EncuentroAleatorio.h` |
| `EstadoJuego` | `enum class` | `Juego.h` |
| `TipoArmaPrueba` | `enum class` | `Juego.h` |
| `TipoRecurso` | `enum class` | `Juego.h` |

**La pregunta probable:** *«¿por qué `TipoEnemigo` es el único clásico?»*

Porque es el único que se convierte a entero. En `GestorOleadas::registroDe()`:

```cpp
const int idBuscado = (int)tipo + 1;   // BASICO(0) → fila 1 del catálogo
```

Con un `enum class` esa línea no compilaría sin `static_cast<int>(tipo)`.

**El seguimiento probable:** *«¿y no convendría `enum class` también ahí?»*

Sí, y se puede defender: obligaría a escribir la conversión. El ejercicio E16 de
la guía propone algo todavía mejor — reemplazar esa cuenta por una función
explícita.

---

## 5. Herencia y composición

**La teoría.** La herencia crea clases derivadas a partir de una base, que
heredan sus atributos y métodos: la relación **«es un»**. La composición hace
que los atributos de una clase sean objetos de otras: la relación **«tiene un»**.

**En el TPI: no hay herencia; composición hay en todos lados.**

```cpp
class Juego {
    Jugador jugador;                      // Juego TIENE UN jugador
    GestorOleadas _gestorOleadas;         // TIENE UN gestor
    Partida _partida{1};                  // TIENE UNA partida
    Escenario _escenario;
    std::vector<Proyectil> _proyectiles;
};
```

**La pregunta casi segura:** *«la herencia es un tema central de la materia.
¿Por qué no la usaron?»*

Porque las cuatro variantes de enemigo **no se diferencian en comportamiento,
solo en datos**. Básico, rápido, resistente y jefe hacen exactamente lo mismo
—perseguir, recibir daño, morir, dar recompensa— con distintos números.

> **Ejemplo bobo:** un Fiat y un Ferrari **no necesitan dos manuales de
> manejo**: los dos arrancan, aceleran y frenan igual. Lo que cambia son los
> números de la ficha técnica. Herencia haría falta si uno **volara**.

Y hay un motivo técnico más fuerte: las clases se escriben al compilar, pero las
filas de `Enemigos.dat` **se leen en ejecución**. Con herencia, agregar una
variante sería escribir una clase y recompilar; como están, es agregar una fila
al archivo.

**Si repreguntan dónde sí usarían herencia:** si un enemigo necesitara moverse
distinto —disparar a distancia, teletransportarse— ahí cambia el
comportamiento, y una jerarquía con un método virtual `actualizar()` empezaría a
pagar.

---

## 6. Asignación dinámica de memoria

**La teoría.** `new` reserva memoria mientras el programa corre y devuelve un
puntero; `delete` la libera. Sirve cuando no se sabe de antemano cuántos
elementos va a haber. Olvidarse del `delete` produce una pérdida de memoria.

> **Ejemplo bobo:** la memoria normal es la **mochila**: entra lo que entra, y
> lo decidiste al comprarla. La dinámica es **alquilar un baúl**: pedís uno
> cuando lo necesitás y lo devolvés al terminar. Si no lo devolvés, lo seguís
> pagando aunque esté vacío — eso es una pérdida de memoria.

**En el TPI no hay un solo `new` ni un solo `delete`.** Y sin embargo se usa
memoria dinámica todo el tiempo: la administran los `std::vector`.

```cpp
std::vector<Enemigo> enemigos;        // crece y se achica durante la partida
std::vector<Proyectil> _proyectiles;
```

**La pregunta probable:** *«¿dónde usan memoria dinámica?»*

En los vectores. Un `vector` reserva en el montículo igual que `new`, pero libera
solo cuando el objeto se destruye. La cantidad de enemigos cambia a cada momento,
así que un arreglo fijo no servía: o sobraba memoria o faltaba lugar.

**El seguimiento probable:** *«¿y por qué no `new` y `delete` directamente?»*

Porque con `new` hay que acordarse del `delete` en **todos** los caminos de
salida, y olvidarse uno es una pérdida. El `vector` da lo mismo sin ese riesgo.

> **Ejemplo bobo:** `new`/`delete` es el baúl que devolvés vos. El `vector` es
> el que **se devuelve solo** cuando te vas.

**Ojo con una confusión fácil:** `char nombre[30]` **no** es memoria dinámica —
es un arreglo fijo que vive dentro del objeto.

---

## 7. Archivos binarios

**La teoría.** Los archivos permiten que los datos sobrevivan al cierre del
programa. Los binarios guardan los bytes tal como están en memoria: `fopen` con
`"rb"`, `"wb"` o `"ab"`, `fwrite` y `fread` indicando el `sizeof` del registro.
Como los registros son de longitud fija, `fseek` y `ftell` permiten contar
cuántos hay.

**En el TPI son cinco** (cuatro hechos, falta `Armas.dat`), y siguen el patrón de
la cátedra casi literalmente:

```cpp
FILE* archivo = fopen(obtenerRutaDatos(RUTA).c_str(), "ab");
if (archivo == NULL) { return false; }
const bool escrito = fwrite(&registro, sizeof(RegistroPartida), 1, archivo) == 1;
fclose(archivo);
```

Y para contar registros, el mismo cálculo del casete del laboratorio —duración
total dividido duración de un tema—:

```cpp
fseek(archivo, 0, SEEK_END);        // al final
const long bytes = ftell(archivo);  // cuántos bytes ocupa
return (int)(bytes / (long)sizeof(RegistroPartida));
```

> **Ejemplo bobo:** si una pila de hojas pesa 500 gramos y cada hoja pesa 5,
> hay 100 hojas. No hace falta contarlas de a una.

**Pregunta probable:** *«¿por qué `"ab"` y no `"wb"`?»* Porque `Partidas.dat` es
un historial: cada partida agrega una fila. `"wb"` vaciaría el archivo.

**Otra:** *«¿por qué `char[30]` y no `std::string`?»* Ver la comparación de la
etiqueta y el papelito, en la Parte 1.

**Una tercera, más fina:** *«¿cuánto ocupa un registro?»*

No es la suma de sus campos. `RegistroEnemigo` tiene 9 campos que suman 62 bytes
y ocupa **64**: 2 de *padding*, porque el compilador alinea los campos.

> **Ejemplo bobo:** es como la **caja de pizza**: la pizza mide 30 cm pero la
> caja mide 32, porque tiene que cerrar bien. Pagás por la caja, no por la
> pizza.

Eso importa porque el tamaño del registro **es** el formato del archivo:
agregar un campo cambia el `sizeof` y vuelve ilegible todo lo guardado antes.

---

# Tabla de respuesta rápida

| Si preguntan por… | Mostrar |
|---|---|
| Clase y objeto | `class Enemigo` contra los 60 que hay en pantalla |
| Encapsulamiento | `vida` privada, `recibirDanio()` pública |
| Constructor | `Enemigo::Enemigo(...)` o la lista de `Arma` |
| Destructor | no hay, y por qué: nada se pide prestado |
| `this` | no se usa; se evitó con el guion bajo |
| `enum` clásico | `TipoEnemigo` y la conversión `(int)tipo + 1` |
| `enum class` | `ResultadoPartida`, `EstadoJuego`, `RarezaMejora`… |
| Herencia | no hay; las variantes son datos, no clases |
| Composición | `Juego` tiene jugador, gestor, partida, escenario |
| Memoria dinámica | los `std::vector`; sin `new` ni `delete` |
| Archivos | `ArchivoPartidas::agregar()` |
| `fseek` / `ftell` | `cantidadRegistros()`: bytes ÷ `sizeof` |
| Arreglos | `char nombre[30]`, y por qué no `std::string` |
| Por referencia | el `&` de `for (Enemigo& e : enemigos)` |

---

# Las tres respuestas para tener memorizadas

**«¿Por qué no usaron herencia?»** Porque las variantes de enemigo se
diferencian en datos, no en comportamiento: un Fiat y un Ferrari no necesitan
dos manuales de manejo. Por eso son filas de un archivo y no clases derivadas, y
así se puede rebalancear el juego sin recompilar.

**«¿Dónde está la memoria dinámica?»** En los `std::vector` de enemigos y
proyectiles, cuya cantidad cambia durante la partida. El vector alquila el baúl
igual que `new`, pero lo devuelve solo; por eso tampoco hace falta destructor.

**«¿Por qué los archivos son binarios y con structs planos?»** Porque `fwrite`
saca una fotocopia de los bytes: solo sirven tipos simples y arreglos de `char`.
Un `std::string` guardaría la dirección de un cajón que en la próxima ejecución
está vacío.
