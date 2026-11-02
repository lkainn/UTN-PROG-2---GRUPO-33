# La teoría de la cursada dentro del TPI

El profesor va a preguntar teoría de Programación 2 **y** dónde se aplica en el
proyecto. Esta guía cruza las dos cosas: para cada tema de la cursada, qué dice
la teoría, dónde está en el código, y la respuesta a la pregunta probable.

Hay tres temas que el proyecto **no usa**, y son los más peligrosos porque
parecen un olvido. No lo son, y conviene tener la respuesta lista:

| Tema | ¿Está en el TPI? |
|---|---|
| Herencia | **no**, y hay un motivo |
| `new` / `delete` | **no**, los `vector` la administran |
| Destructores propios | **no**, no hacen falta |
| Puntero `this` | **no explícitamente**, pero está en cada método |

---

## 1. POO: clase, objeto, encapsulamiento

**La teoría.** Una clase es el molde; un objeto es cada ejemplar creado a partir
de ese molde. El encapsulamiento consiste en declarar los atributos `private` y
dar acceso controlado con métodos `public` (*getters* y *setters*), para que
nadie pueda dejar el objeto en un estado inválido.

**En el TPI.** Todas las clases siguen la regla: ningún atributo es público.
El caso más claro es `Enemigo`:

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
reglas. Con `recibirDanio()`, la lógica de cómo cambia la vida —incluido el
tope en cero— vive adentro de la clase y vale para todos los que la usen.

**Un matiz que suma:** no todos los atributos tienen *setter*. `Enemigo` no
tiene `setVida()`, solo `recibirDanio()`. Un *setter* para cada atributo
equivale a dejarlos públicos con pasos extra; los métodos deben representar lo
que la entidad **hace**, no exponer sus variables una por una.

---

## 2. Constructores y destructores

**La teoría.** El constructor se ejecuta solo al crear el objeto y deja sus
atributos en un estado válido. El destructor (`~Clase()`) se ejecuta al
destruirlo, y sirve para liberar lo que el objeto haya reservado.

**En el TPI.** Constructores hay en todas las clases. Dos formas conviven:

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

**Destructores: el proyecto no define ninguno.** Y está bien.

**La pregunta probable:** *«¿por qué ninguna clase tiene destructor?»*

Porque ninguna reserva recursos que haya que liberar a mano. Los `std::vector`
liberan su memoria solos cuando el objeto muere, y los `FILE*` se cierran con
`fclose()` en la misma función que los abrió. Un destructor vacío no agregaría
nada. Si alguna clase pasara a usar `new`, ahí sí haría falta.

---

## 3. El puntero `this`

**La teoría.** De los apuntes: *«`this` es un puntero que contiene la dirección
del objeto que llama a un método»*. Existe porque los datos se guardan una vez
por objeto, pero el código del método es uno solo y compartido: `this` es lo que
le permite saber sobre cuál objeto trabajar. Es oculto —no hace falta
escribirlo— pero se puede usar explícitamente con el operador `->`.

**En el TPI: no aparece escrito en ninguna parte.** Está implícito en cada
método.

**La pregunta probable:** *«¿usaron `this`? ¿dónde lo necesitarían?»*

La respuesta honesta: no explícitamente, porque se evitó el caso que lo hace
necesario. Cuando un parámetro se llama igual que un atributo, hay que
distinguirlos, y `this->` es una de las dos formas:

```cpp
// con this: el parámetro y el atributo se llaman igual
Enemigo::Enemigo(int vida) { this->vida = vida; }

// lo que hicimos: el parámetro lleva guion bajo
Enemigo::Enemigo(int _vida) { vida = _vida; }
```

Las dos son válidas. Sin ninguna de las dos, `vida = vida;` se asigna el
parámetro a sí mismo, el atributo nunca se carga, **y compila sin avisar**.

---

## 4. Enumeradores: `enum` contra `enum class`

**La teoría.** Un `enum` da nombre a un conjunto cerrado de valores, en lugar de
usar enteros sueltos sin significado. El `enum` clásico expone sus nombres en el
ámbito donde se declara —dos enumeraciones con un valor homónimo chocan— y
convierte a `int` implícitamente. El `enum class` de C++11 encierra cada valor
en el ámbito de su tipo, obliga a escribir `Tipo::VALOR`, y **no** convierte a
`int` sin un `static_cast` explícito.

**En el TPI hay siete, y justo de los dos tipos:**

| Enumerador | Forma | Dónde |
|---|---|---|
| `TipoEnemigo` | `enum` clásico | `Enemigo.h` |
| `ResultadoPartida` | `enum class` | `Partida.h` |
| `RarezaMejora` | `enum class` | `InventarioArmas.h` |
| `TipoEncuentro` | `enum class` | `EncuentroAleatorio.h` |
| `EstadoJuego` | `enum class` | `Juego.h` |
| `TipoArmaPrueba` | `enum class` | `Juego.h` |
| `TipoRecurso` | `enum class` | `Juego.h` |

**La pregunta probable:** *«¿por qué `TipoEnemigo` es el único `enum` clásico?»*

Porque es el único que se convierte a entero. En `GestorOleadas::registroDe()`:

```cpp
const int idBuscado = (int)tipo + 1;   // BASICO(0) → fila 1 del catálogo
```

Con un `enum class` esa línea no compilaría sin `static_cast<int>(tipo)`. La
conversión implícita es cómoda acá, pero es exactamente lo que `enum class`
previene en el resto del proyecto.

**El seguimiento probable:** *«¿y no sería mejor `enum class` también ahí?»*

Sí, y se puede defender: obligaría a escribir la conversión, y hay un ejercicio
en la guía (E16) que propone reemplazar esa cuenta por una función explícita,
que es más robusta todavía.

---

## 5. Herencia y composición

**La teoría.** La herencia crea clases derivadas a partir de una clase base, que
heredan sus atributos y métodos. Es la relación **«es un»**: un Alumno *es una*
Persona. La composición es la relación **«tiene un»**: los atributos de una
clase son objetos de otras clases; un Examen *tiene una* Fecha, una Materia y un
Alumno.

**En el TPI: no hay herencia. Hay composición por todos lados.** `Juego`
contiene un `Jugador`, un `GestorOleadas`, una `Partida`, un `Escenario`, cuatro
`Arma`, un `InventarioArmas`, un `vector<Proyectil>`:

```cpp
class Juego {
    Jugador jugador;
    GestorOleadas _gestorOleadas;
    Partida _partida{1};
    Escenario _escenario;
    std::vector<Proyectil> _proyectiles;
};
```

**La pregunta probable, y es casi segura:** *«la herencia es un tema central de
la materia. ¿Por qué no la usaron?»*

La respuesta: porque las cuatro variantes de enemigo **no se diferencian en
comportamiento, solo en datos**. Básico, rápido, resistente y jefe hacen
exactamente lo mismo —perseguir, recibir daño, morir, dar recompensa— con
distintos números. La herencia sirve cuando las derivadas *hacen* cosas
distintas, no cuando solo *valen* distinto.

Una jerarquía `Enemigo` → `EnemigoRapido` con cuatro clases que solo cambian
constantes sería más código para el mismo resultado, y además imposibilitaría lo
que sí se hizo: **leer las variantes de un archivo**. Las clases se escriben en
tiempo de compilación; las filas de `Enemigos.dat` se leen en ejecución, y por
eso se puede agregar o rebalancear una variante sin recompilar.

**Si repreguntan dónde sí usarían herencia:** si en algún momento un enemigo
necesitara moverse distinto —uno que dispara a distancia, otro que se teletransporta—
ahí el comportamiento cambia y la herencia (con un método virtual `actualizar`)
empezaría a pagar.

---

## 6. Asignación dinámica de memoria

**La teoría.** `new` reserva memoria en tiempo de ejecución y devuelve un
puntero; `delete` la libera. Sirve cuando no se sabe de antemano cuántos
elementos va a haber. Olvidarse del `delete` produce una pérdida de memoria.

**En el TPI no hay un solo `new` ni un solo `delete`.** Y sin embargo el
proyecto usa memoria dinámica todo el tiempo: la administran los `std::vector`.

```cpp
std::vector<Enemigo> enemigos;     // crece y se achica durante la partida
std::vector<Proyectil> _proyectiles;
```

**La pregunta probable:** *«¿dónde usan memoria dinámica?»*

En los vectores. Un `vector` reserva memoria en el heap igual que `new`, pero la
libera solo cuando el objeto se destruye. La cantidad de enemigos y de
proyectiles cambia a cada momento, así que un arreglo fijo no servía: o sobraba
memoria o faltaba lugar.

**El seguimiento probable:** *«¿y por qué no `new` y `delete` directamente?»*

Porque con `new` hay que acordarse del `delete` en todos los caminos de salida
—incluido el de una excepción— y olvidarse uno solo es una pérdida de memoria.
El `vector` da lo mismo sin ese riesgo. Es el mismo criterio por el que no hace
falta destructor: nadie administra recursos a mano.

**Ojo con una confusión fácil:** `Enemigo` tiene `char nombre[30]`, que **no** es
memoria dinámica — es un arreglo de tamaño fijo dentro del objeto. Se eligió
así justamente porque va a un archivo binario.

---

## 7. Archivos binarios

**La teoría.** Los archivos permiten que los datos sobrevivan al cierre del
programa. Los binarios guardan los bytes tal como están en memoria: se abren con
`fopen` en modo `"rb"`, `"wb"` o `"ab"`, se escriben con `fwrite` y se leen con
`fread`, siempre indicando `sizeof` del registro. Como los registros son de
longitud fija, `fseek` y `ftell` permiten moverse por posición y calcular cuántos
hay: tamaño del archivo dividido tamaño del registro.

**En el TPI son cinco** (cuatro hechos, falta `Armas.dat`), y siguen el patrón de
la cátedra casi literalmente.

Alta de un registro, igual que el ejemplo de `examenes.dat`:

```cpp
FILE* archivo = fopen(obtenerRutaDatos(RUTA).c_str(), "ab");
if (archivo == NULL) { return false; }
const bool escrito = fwrite(&registro, sizeof(RegistroPartida), 1, archivo) == 1;
fclose(archivo);
```

Y contar registros, que es exactamente el cálculo del casete del laboratorio
—duración total dividido duración de un tema—:

```cpp
fseek(archivo, 0, SEEK_END);        // al final
const long bytes = ftell(archivo);  // cuántos bytes ocupa
return (int)(bytes / (long)sizeof(RegistroPartida));
```

**La pregunta probable:** *«¿por qué `"ab"` y no `"wb"`?»*

Porque `Partidas.dat` es un **historial**: cada partida agrega una fila y
ninguna pisa a la anterior. `"ab"` escribe al final; `"wb"` vacía el archivo y
lo reescribe desde cero. Los catálogos (`Enemigos.dat`, `Mejoras.dat`) sí usan
`"wb"`, porque se reemplazan enteros.

**La otra pregunta probable:** *«¿por qué `char[30]` y no `std::string`?»*

Porque un `string` guarda internamente un puntero a otra parte de la memoria. Al
volcarlo crudo con `fwrite` se guardaría una dirección, que en la próxima
ejecución no significa nada. Un arreglo de tamaño fijo se guarda tal cual.

**Una tercera, más fina:** *«¿cuánto ocupa un registro?»*

No es la suma de sus campos. `RegistroEnemigo` tiene 9 campos que suman 62 bytes
y ocupa **64**: 2 bytes de *padding*, porque el compilador alinea los campos. Eso
importa porque el tamaño del registro **es** el formato del archivo: agregar un
campo cambia `sizeof` y vuelve ilegible todo lo guardado antes.

---

## Tabla de respuesta rápida

| Si preguntan por… | Mostrar |
|---|---|
| Encapsulamiento | `Enemigo`: `vida` privada, `recibirDanio()` pública |
| Constructor | `Enemigo::Enemigo(...)` o la lista de inicialización de `Arma` |
| Destructor | no hay, y por qué: nada administra recursos a mano |
| `this` | no se usa; se evitó con el guion bajo en los parámetros |
| `enum` clásico | `TipoEnemigo`, y la conversión `(int)tipo + 1` |
| `enum class` | `ResultadoPartida`, `EstadoJuego`, `RarezaMejora`… |
| Herencia | no hay; explicar por qué las variantes son datos y no clases |
| Composición | `Juego` contiene jugador, gestor, partida, escenario, armas |
| Memoria dinámica | los `std::vector`; sin `new` ni `delete` |
| Archivos | `ArchivoPartidas::agregar()` y `cantidadRegistros()` |
| `fseek` / `ftell` | `cantidadRegistros()`: bytes ÷ `sizeof(registro)` |
| Arreglos | `char nombre[30]`, y por qué no `std::string` |

---

## Las tres respuestas que conviene tener memorizadas

**«¿Por qué no usaron herencia?»** Porque las variantes de enemigo se
diferencian en datos, no en comportamiento. Hacen lo mismo con distintos
números, así que son filas de un archivo y no clases derivadas. Eso además
permite rebalancear el juego sin recompilar.

**«¿Dónde está la memoria dinámica?»** En los `std::vector` de enemigos y
proyectiles, cuya cantidad cambia durante la partida. El vector reserva en el
heap como `new`, pero libera solo; por eso tampoco hace falta destructor.

**«¿Por qué los archivos son binarios y con structs planos?»** Porque un
`fwrite` copia los bytes tal cual: solo sirven tipos simples y arreglos de
`char`. Un `std::string` guardaría un puntero inútil. Y como el registro es de
longitud fija, se puede contar con `fseek`/`ftell` y moverse por posición.
