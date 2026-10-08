# Guía de proyectiles y texturas - Proyecto TPI

Esta guía describe la primera integración de pistola y metralleta. La ampliación
a cuatro texturas y la selección por ID ya están implementadas; consultar la
[guía de gráficos de armas nuevas](guia_graficos_armas_nuevas.md) para el estado
actual de esos puntos. Los fragmentos de dos texturas y selección por dimensiones
que aparecen debajo corresponden a la implementación anterior.



Archivos principales:

- [Proyectil.h](../include/Proyectil.h) y [Proyectil.cpp](../src/Proyectil.cpp).
- [AspectoProyectiles.h](../include/AspectoProyectiles.h) y
  [AspectoProyectiles.cpp](../src/AspectoProyectiles.cpp).
- [Arma.cpp](../src/Arma.cpp) y [Juego.cpp](../src/Juego.cpp).


Guías relacionadas: [escenario](guia_escenario.md) y
[cofres y estaciones](guia_cofres_estaciones.md).

## 1. Separación de responsabilidades

| Clase | Responsabilidad |
|---|---|
| `Arma` | Controla cadencia y crea un proyectil dirigido al objetivo. |
| `Proyectil` | Guarda posición, velocidad, dimensiones, daño y alcance restante. |
| `AspectoProyectiles` | Carga texturas y dibuja el proyectil orientado. |
| `Juego` | Conserva varios proyectiles y coordina actualización, impactos y dibujo. |

La textura no decide cuánto daño hacer. El movimiento no depende del tamaño
del PNG. Esta separación permite cambiar la apariencia sin cambiar el combate.

## 2. Atributos lógicos del proyectil

```cpp
Posicion _posicion;
float _velocidadX;
float _velocidadY;
float _ancho;
float _alto;
int _dano;
bool _activo;
float _distanciaRestante = std::numeric_limits<float>::infinity();
```

La posición representa la esquina superior izquierda de su forma de colisión.
Las velocidades son componentes en píxeles por segundo. El ancho y el alto
definen el rectángulo lógico, no las dimensiones originales de la imagen.

El constructor valida posiciones, velocidades y dimensiones finitas. Una
dimensión inválida queda en cero y el proyectil no se considera activo.

La distancia inicia en infinito para conservar el comportamiento de proyectiles
construidos directamente en pruebas antiguas. Los disparos creados por `Arma`
reciben un alcance finito mediante `limitarAlcance`.

## 3. Movimiento y alcance restante

Fragmento de `actualizar`:

```cpp
const float paso = std::hypot(_velocidadX, _velocidadY) * segundos;
const float fraccion = std::min(1.0f, _distanciaRestante / paso);
_posicion.x += _velocidadX * segundos * fraccion;
_posicion.y += _velocidadY * segundos * fraccion;
```

Antes de estas cuentas se rechazan tiempo inválido, proyectil inactivo y
desplazamiento nulo. `hypot` calcula la longitud del vector de velocidad.

Ejemplo: una bala avanza a 800 píxeles por segundo y el fotograma dura 0,05 s.
El paso sería 40 píxeles. Si solo quedan 10 de alcance, la fracción es
`10 / 40 = 0,25`, por lo que solo avanza esos 10 píxeles.

```cpp
_distanciaRestante = std::max(0.0f, _distanciaRestante - paso);
if (_distanciaRestante <= 0.0f)
{
    desactivar();
}
```

La configuración actual de ambas armas en `Juego` usa alcance de 600 píxeles.
Al agotarse, el proyectil deja de participar en impactos. También se desactiva
al impactar o salir del área visible usada por la integración actual.

## 4. Atributos visuales y carga única

```cpp
std::array<sf::Texture, 2> _texturas;
std::array<sf::IntRect, 2> _recortes;
std::array<bool, 2> _disponibles;
```

Los tres arreglos utilizan el mismo índice: 0 para pistola y 1 para metralleta.
`std::array` contiene exactamente dos elementos, inicializados en el constructor.

```cpp
_disponibles[0] = cargar(0, carpeta, "pistola_v1.png");
_disponibles[1] = cargar(1, carpeta, "metralleta_v1.png");
```

La textura se comparte entre todos los disparos de esa arma. No se reserva
una textura por bala ni se recarga desde disco en cada fotograma.

`explicit` en el constructor impide convertir implícitamente un texto de ruta
en un objeto `AspectoProyectiles`. El parámetro de carpeta tiene un valor por
defecto y permite probar una carpeta inexistente sin mover los recursos reales.

## 5. Diferencia entre sf::Image y sf::Texture

`sf::Image` permite consultar píxeles en memoria. Se utiliza temporalmente para
analizar transparencia. `sf::Texture` conserva el recurso gráfico para dibujar.

La carga busca en `recursos/proyectiles` y, desde `bin/Debug`, en
`../../recursos/proyectiles`. Se comprueba la existencia antes de intentar abrir.

Las imágenes originales son grandes:

| Arma | PNG original | Tamaño dibujado |
|---|---|---|
| Pistola | 1774 × 887 | 28 × 14 |
| Metralleta | 1916 × 821 | 14 × 6 |

No son sprites nativos de esas pequeñas dimensiones: se selecciona su región
visible y se adapta a la figura durante el dibujo.

## 6. Canal alfa y región visible

Cada píxel tiene rojo, verde, azul y alfa. Alfa 0 significa completamente
transparente y alfa 255 completamente opaco.

```cpp
if (imagen.getPixel({x, y}).a >= 32)
{
    izquierda = std::min(izquierda, x);
    arriba = std::min(arriba, y);
    derecha = std::max(derecha, x);
    abajo = std::max(abajo, y);
}
```

Se recorre la imagen para encontrar los extremos de los píxeles suficientemente
visibles. El umbral 32 ignora residuos casi transparentes alrededor de la bala.

Si incluyéramos márgenes grandes dentro del tamaño final, la bala visible sería
muy pequeña. El análisis de extremos evita ese problema.

## 7. Rectángulo de recorte y static_cast

```cpp
_recortes[indice] = sf::IntRect(
    {static_cast<int>(izquierda), static_cast<int>(arriba)},
    {static_cast<int>(derecha - izquierda + 1),
     static_cast<int>(abajo - arriba + 1)});
```

La primera pareja es la esquina superior izquierda y la segunda es el tamaño.
El `+1` incluye ambos extremos: de la columna 10 a la 19 hay 10 columnas,
no 9. `static_cast<int>` convierte los índices sin signo al tipo del rectángulo.

El archivo PNG no se modifica ni se vuelve a guardar. `setTextureRect` indica
qué región de la textura mostrar. Si no se encuentra ningún píxel con alfa
suficiente, se utiliza el dibujo de respaldo.

## 8. Reconocer el aspecto del arma

```cpp
const bool metralleta = forma.ancho == 14.0f && forma.alto == 6.0f;
const std::size_t indice = metralleta ? 1 : 0;
```

El operador `? :` elige 1 cuando la condición es verdadera y 0 cuando es falsa.
`std::size_t` es el tipo habitual para índices y tamaños de colecciones.

La selección consulta las dimensiones del propio disparo; cambiar el arma
equipada no cambia el aspecto de las balas que ya están viajando.

Limitación actual: otras dimensiones utilizan la textura de pistola. Si futuras
armas comparten tamaños, habrá que incorporar un identificador visual explícito.

## 9. Centro del dibujo y centro de colisión

```cpp
sf::RectangleShape figura({forma.ancho, forma.alto});
figura.setOrigin({forma.ancho / 2.0f, forma.alto / 2.0f});
figura.setPosition({forma.x + forma.ancho / 2.0f,
                    forma.y + forma.alto / 2.0f});
```

`setOrigin` coloca el punto de giro en el centro. `setPosition` ubica ese punto
en el centro del rectángulo lógico.

```text
Esquina lógica: (100, 200)
Tamaño: 28 × 14
Centro: (114, 207)
```

Sin ajustar el origen, la figura giraría alrededor de una esquina y parecería
desplazarse respecto de su posición lógica.

## 10. Ángulo de movimiento: atan2 y radianes

```cpp
float Proyectil::obtenerAngulo() const
{
    return std::atan2(_velocidadY, _velocidadX);
}
```

`atan2` calcula el ángulo del vector, considerando su cuadrante. El orden es
componente Y primero y X después. No hace falta dividir Y por X, por lo que
también funciona al disparar verticalmente.

| Velocidad | Ángulo aproximado | Dirección en pantalla |
|---|---|---|
| (100, 0) | 0 rad | Derecha |
| (0, 100) | 1,57 rad | Abajo |
| (-100, 0) | 3,14 rad | Izquierda |
| (100, 100) | 0,79 rad | Diagonal abajo-derecha |

En SFML, Y crece hacia abajo. El método es `const` porque solo consulta velocidades.

```cpp
figura.setRotation(sf::radians(proyectil.obtenerAngulo()));
```

`sf::radians` expresa la unidad que recibe SFML. Las imágenes fueron diseñadas
apuntando a la derecha: esa es la orientación de cero radianes.

## 11. Textura, bordes definidos y respaldo

```cpp
figura.setTexture(&_texturas[indice]);
figura.setTextureRect(_recortes[indice]);
```

La figura referencia una textura que pertenece al módulo. Es una figura local
y la textura permanece viva durante el dibujo. `setSmooth(false)` evita suavizado.

Si la textura no está disponible, se dibuja un rectángulo amarillo para pistola
o naranja para metralleta. Si el proyectil está inactivo, no se dibuja nada.

El dibujo recibe `const Proyectil&`: no copia ni modifica el proyectil original.

## 12. Integración con Juego y colisiones

```cpp
for (const Proyectil& proyectil : _proyectiles)
{
    _aspectoProyectiles.dibujar(ventana, proyectil);
}
```

Se utiliza la vista del mundo, igual que para jugador y enemigos. Después se
restaura la vista fija de la interfaz. El módulo fue registrado en Code::Blocks
y en los archivos de compilación que incluyen `Juego`.

El giro modifica la apariencia, no el rectángulo de colisión. Este conserva su
ancho y alto originales alineados a los ejes. Por eso, una bala vertical no tiene
una colisión rotada exactamente según sus píxeles. Es una simplificación actual.

No se cambiaron daño, cadencia ni velocidad al agregar las texturas. Los recursos
se administran por duración de objetos; no se introdujeron reservas manuales.
