# Guía de escenario y la cámara - Proyecto TPI



Archivos principales:

- [Escenario.h](../include/Escenario.h) y [Escenario.cpp](../src/Escenario.cpp).
- [Juego.h](../include/Juego.h) y [Juego.cpp](../src/Juego.cpp).
- [Configuracion.h](../include/Configuracion.h).
- [TiposCompartidos.h](../include/TiposCompartidos.h).
- [Pruebas de cámara](../pruebas/Camara.cpp).



## 1. Responsabilidad de Escenario

`Escenario` carga la imagen del suelo, la dibuja repetida y administra la vista
que sigue al jugador. `Juego` decide cuándo actualizar y dibujar esa vista.

La imagen del suelo no contiene objetos con colisión: sus manchas y fragmentos
son detalles de una sola textura. Estaciones y cofres pertenecen a otro módulo.

## 2. Atributos y tipos de SFML

```cpp
sf::Texture _texturaSuelo;
sf::RectangleShape _fondo;
sf::View _vista;
bool _fondoDisponible;
```

| Atributo | Función |
|---|---|
| `_texturaSuelo` | Conserva la imagen preparada para dibujar. |
| `_fondo` | Rectángulo que muestra esa textura. |
| `_vista` | Cámara que determina qué parte del mundo vemos. |
| `_fondoDisponible` | Indica si la carga tuvo éxito. |

Una textura no tiene por sí sola una posición en el mundo. La posición y el
tamaño pertenecen a la figura que utiliza esa textura.

## 3. Constructor y lista de inicialización

Fragmento del constructor:

```cpp
_fondo({static_cast<float>(Configuracion::ANCHO_VENTANA),
        static_cast<float>(Configuracion::ALTO_VENTANA)}),
_vista(sf::FloatRect({0.0f, 0.0f}, _fondo.getSize())),
_fondoDisponible(cargarSuelo())
```

Los atributos se construyen antes de entrar al cuerpo del constructor, en el
orden en que están declarados en el encabezado. Aquí `_fondo` ya existe cuando
se consulta su tamaño para construir `_vista`.

`static_cast<float>` convierte las dimensiones enteras a valores decimales,
que son los tipos que utiliza la posición y el tamaño de la figura.

El fondo mide actualmente 1920 × 1080. El PNG original mide 1672 × 941 y se
adapta al rectángulo; no son necesariamente las mismas dimensiones.

## 4. Carga de la imagen y rutas relativas

```cpp
constexpr std::array<const char*, 2> RUTAS_SUELO{{
    "recursos/entorno/suelo_chatarra_v1.png",
    "../../recursos/entorno/suelo_chatarra_v1.png"
}};
```

`std::array` guarda una cantidad fija de elementos. En este caso hay dos rutas.
`const char*` permite referenciar el texto de cada ruta sin modificarlo.

Las rutas se resuelven desde la carpeta de trabajo de la ejecución, no desde
la ubicación del archivo `.cpp`. La primera sirve desde la raíz del proyecto;
la segunda, desde `bin/Debug`. `..` significa subir una carpeta.

```cpp
std::ifstream archivo(ruta, std::ios::binary);
if (archivo.good() && _texturaSuelo.loadFromFile(ruta))
{
    return true;
}
```

`ifstream` comprueba el acceso al archivo. `loadFromFile` carga la imagen.
Con `&&`, la segunda condición solo se evalúa si la primera es verdadera.
El archivo se cierra automáticamente al salir del bloque.

La imagen se carga durante la construcción, no en cada fotograma. Si ninguna
ruta funciona, se conserva un fondo liso oscuro.

## 5. Asociar la textura con la figura

```cpp
_texturaSuelo.setSmooth(false);
_fondo.setTexture(&_texturaSuelo, true);
```

`setSmooth(false)` evita el filtrado suave, conservando bordes definidos.
`&_texturaSuelo` obtiene la dirección de la textura: la figura guarda una
referencia a ella mediante un puntero. El argumento `true` ajusta el rectángulo
de textura para seleccionar la imagen completa.

La textura debe vivir mientras se utilice la figura. Por eso ambos atributos
pertenecen al mismo `Escenario`.

## 6. Por qué se impide copiar Escenario

```cpp
Escenario(const Escenario&) = delete;
Escenario& operator=(const Escenario&) = delete;
```

`= delete` prohíbe esas operaciones. Una copia automática podría dejar la
figura de la copia apuntando a la textura del objeto original.

El objeto se conserva directamente como atributo de `Juego`. No se usa `new`
ni `delete` para administrar su vida. Los recursos se liberan al destruirse.

## 7. Coordenadas del mundo y coordenadas de pantalla

Una estación puede permanecer en `{2500, 800}` aunque no esté en pantalla.
Esas son sus coordenadas del mundo. La cámara decide dónde aparece en la ventana.

```cpp
_escenario.actualizar(jugador.getPosicion());
```

Dentro de `Escenario::actualizar`:

```cpp
if (std::isfinite(posicionJugador.x) && std::isfinite(posicionJugador.y))
{
    _vista.setCenter({posicionJugador.x, posicionJugador.y});
}
```

`std::isfinite` rechaza infinitos y `NaN`. Con una posición válida, el centro
de la cámara sigue al jugador. No desplazamos todas las entidades manualmente.

Ejemplo: el jugador avanza 100 píxeles a la derecha. La cámara también avanza
100 píxeles; un objeto quieto parece moverse 100 píxeles a la izquierda en pantalla.

La llamada temporal `jugador.actualizar(false)` permite caminar fuera del
rectángulo fijo de la ventana. Pertenece a la integración autorizada con Jugador.

## 8. Obtener el área visible

```cpp
return {centro.x - tamanio.x / 2.0f,
        centro.y - tamanio.y / 2.0f,
        tamanio.x, tamanio.y};
```

El centro no es la esquina superior izquierda. Para obtener esa esquina se
resta la mitad del ancho y del alto.

```text
Vista: 1920 × 1080
Centro: (960, 540)  → esquina: (0, 0)
Centro: (1060, 540) → esquina: (100, 0)
```

La función devuelve una `FormaColision` como estructura de datos. Eso no
convierte el borde de la cámara en una pared del mundo.

## 9. Elegir el inicio de las baldosas

```cpp
const float inicioX = std::floor(area.x / tamanio.x) * tamanio.x;
const float inicioY = std::floor(area.y / tamanio.y) * tamanio.y;
```

`floor` redondea hacia abajo. Con ancho 1920:

| `area.x` | `floor(area.x / 1920)` | `inicioX` |
|---|---|---|
| 100 | 0 | 0 |
| 2000 | 1 | 1920 |
| -100 | -1 | -1920 |

Así el suelo queda anclado a una cuadrícula del mundo, incluso en coordenadas
negativas. Redondear hacia cero produciría un inicio incorrecto en el último caso.

## 10. Dibujar cuatro baldosas

```cpp
sf::RectangleShape baldosa = _fondo;
for (int fila = 0; fila < 2; ++fila)
{
    for (int columna = 0; columna < 2; ++columna)
    {
        baldosa.setPosition({inicioX + columna * tamanio.x,
                             inicioY + fila * tamanio.y});
        destino.draw(baldosa);
    }
}
```

Los ciclos dibujan una cuadrícula de 2 × 2. Con el tamaño actual de la vista,
alcanza para cubrirla cuando cruza los bordes de una baldosa.

Copiar esta figura local es seguro: todas las copias usan la textura que
todavía pertenece al escenario. No se crean cuatro texturas nuevas.

La firma `dibujar(sf::RenderTarget& destino) const` recibe el destino por
referencia y no modifica los atributos de `Escenario`. Sí puede dibujar sobre
el destino recibido. `RenderTarget` permite recibir una ventana o una textura
de renderizado, útil para pruebas.

## 11. Orden de dibujo e interfaz fija

En `Juego`, el orden simplificado es:

```cpp
ventana.setView(_escenario.obtenerVista());
_escenario.dibujar(ventana);
// Dibujar encuentros, recursos, enemigos, jugador y proyectiles.
ventana.setView(ventana.getDefaultView());
dibujarInterfaz();
```

Lo que se dibuja después puede tapar lo anterior. El suelo va primero.
Restaurar la vista predeterminada mantiene la interfaz fija en pantalla.

`obtenerVista()` devuelve `const sf::View&`: consulta la vista original sin
copiarla y sin permitir modificarla mediante esa referencia.

## 12. Relación con las zonas de cofres y estaciones

Las baldosas gráficas miden 1920 × 1080. Las zonas de encuentros miden
1200 × 1200. Son dos divisiones distintas con responsabilidades diferentes.
Ambas usan coordenadas del mismo mundo; una cubre visualmente el suelo y la
otra decide dónde existen objetos interactivos.
