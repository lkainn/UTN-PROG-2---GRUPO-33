# Guía de gráficos de escopeta y cañón de energía

## 1. Archivos y correspondencia

| ID de arma | Índice | Arma en `recursos/armas/` | Proyectil en `recursos/proyectiles/` |
|---|---|---|---|
| 1 | 0 | `pistola_concepto_v1.png` | `pistola_v1.png` |
| 2 | 1 | `metralleta_concepto_v1.png` | `metralleta_v1.png` |
| 3 | 2 | `escopeta_concepto_v1.png` | `escopeta_v1.png` |
| 4 | 3 | `canon_energia_concepto_v1.png` | `canon_energia_v1.png` |

Cada clase conserva tres `std::array` de cuatro elementos: texturas, recortes
y estados de carga. Los tres usan el mismo índice. Para un ID válido,
`indice = idArma - 1`: la escopeta tiene ID 3 y ocupa la posición 2.

Las texturas se cargan una vez en el constructor y pertenecen al objeto por
valor. Las figuras que las usan son temporales durante el dibujo. Los recursos
se liberan automáticamente al destruir el objeto (RAII); no hay reservas
manuales ni una textura nueva por disparo. El tamaño fijo de cuatro elementos
resuelve el catálogo actual sin introducir memoria dinámica manual.

## 2. Consulta por ID y compatibilidad

Ambas clases agregaron `texturaDisponible(int idArma) const`. Devuelve `false`
si el ID está fuera de 1 a 4 o si su textura no pudo cargarse.

```cpp
return idArma >= 1 && idArma <= 4 && _disponibles[idArma - 1];
```

`&&` evalúa de izquierda a derecha y se detiene ante una condición falsa.
Por eso no se consulta el arreglo cuando el ID es inválido.

La sobrecarga anterior con `bool` sigue disponible: `false` representa pistola
y `true` metralleta. Ahora delega en la consulta por ID. Para las armas nuevas
se pasan los enteros `3` o `4`, no valores booleanos.

`AspectoArmas::dibujar` descarta IDs inválidos, posiciones no finitas y ángulos
no finitos. En proyectiles, los IDs válidos seleccionan directamente la textura.
Para conservar compatibilidad, ID 0 con dimensiones 14x6 utiliza metralleta;
los demás IDs fuera de rango utilizan pistola. Este respaldo visual no convierte
un ID inválido en un arma válida del catálogo.

## 3. Carga y recorte

El mecanismo existente se reutiliza para los cuatro PNG: busca en la carpeta
de recursos y luego en `../../recursos/...`, para ejecutar desde `bin/Debug`.
Si la carga falla o no hay píxeles con alfa suficiente, la textura queda
marcada como no disponible.

Se calculan los límites de los píxeles con alfa mayor o igual a 32. El recorte
evita que los márgenes transparentes reduzcan el tamaño visible del dibujo.
`setTextureRect` selecciona esa región sin modificar el PNG original.
`setSmooth(false)` conserva bordes definidos al reducir la imagen.

Son imágenes ampliadas con aspecto pixelado, no sprites creados originalmente
en las dimensiones pequeñas del juego. Tampoco son hojas de animación.

## 4. Tamaño, pivote y orientación del arma

| Arma | Ancho dibujado | Pivote horizontal como fracción del ancho | Alto sin PNG |
|---|---|---|---|
| Pistola | 48 px | 0,23 | 24 px |
| Metralleta | 72 px | 0,38 | 18 px |
| Escopeta | 60 px | 0,30 | 18 px |
| Cañón de energía | 76 px | 0,25 | 18 px |

Con textura, el alto se calcula proporcionalmente al recorte:

```cpp
alto = ancho * recorteAlto / recorteAncho;
```

El pivote es el punto alrededor del cual gira la figura. Su componente vertical
es `alto * 0.60f`; la horizontal varía para acompañar la empuñadura de cada
silueta. Las tablas `ANCHOS` y `PIVOTES` reemplazan el bloque especial que
dibujaba las armas nuevas como rectángulos provisionales.

El pivote queda a 8 píxeles del centro del jugador en la dirección del apuntado.
La figura gira usando el ángulo del objetivo automático. Cuando `cos(angulo)`
es negativo, `setScale({1.0f, -1.0f})` refleja la figura verticalmente para
mantener su orientación al apuntar a la izquierda. Este tratamiento ahora es
común a las cuatro armas. El origen lógico de los disparos sigue siendo el
centro del jugador, no la boca del dibujo del arma.

## 5. Dibujo de proyectiles

| Arma de origen | Dimensiones dibujadas y de colisión |
|---|---|
| Pistola | 28x14 px |
| Metralleta | 14x6 px |
| Escopeta | 8x8 px |
| Cañón de energía | 40x10 px |

Las dimensiones proceden de la forma de colisión del proyectil. Su PNG se
adapta a ese rectángulo, aunque la proporción original sea distinta. A
diferencia de las armas, aquí no se calcula un alto proporcional a la imagen.

El dibujo se centra en la forma de colisión y gira según el ángulo de la
velocidad. La textura se elige con el ID almacenado en cada proyectil; equipar
otra arma no transforma los disparos que ya están viajando. Un proyectil
inactivo no se dibuja.

El giro es visual: la colisión conserva su rectángulo alineado a los ejes.
No se comprueba una colisión exacta contra los píxeles transparentes del PNG.

## 6. Respaldo sin imágenes

Si falta un PNG, el juego conserva una figura rectangular del tamaño previsto.
La escopeta usa marrón `(190, 130, 60)` y su perdigón ámbar `(255, 200, 110)`.
El cañón y su proyectil usan cian `(70, 220, 255)`. Los colores de pistola y
metralleta se conservaron. La ausencia de una textura no impide el combate.
