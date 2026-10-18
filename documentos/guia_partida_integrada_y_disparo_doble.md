# Guía de integración de partida y disparo de dos armas

## 1. Archivos involucrados

| Archivo | Responsabilidad en este incremento |
|---|---|
| `include/Juego.h` | Estado de resultado, instancia de partida, ángulos por slot y métodos de coordinación. |
| `src/Juego.cpp` | Actualización de estadísticas, desenlace, reinicio, presentación y disparos de ambos slots. |
| `include/AspectoArmas.h` | Consulta de la posición de la boca del arma. |
| `src/AspectoArmas.cpp` | Transformación compartida entre dibujo y cálculo de la boca. |
| `pruebas/PartidaIntegrada.cpp` | Pruebas del ciclo completo y del origen de los disparos. |
| `pruebas/probar_partida_integrada.bat` | Compilación y ejecución de la nueva prueba. |
| Pruebas de combate y gráficos existentes | Adaptación a las identidades de enemigos y al disparo simultáneo. |
| Scripts de pruebas que compilan `Juego` | Incorporación de `src/Partida.cpp` al enlace. |

## 2. Conectar las estadísticas con el ciclo de Juego

`Juego` mantiene una partida en memoria y recuerda cuántas bajas ya informó:

```cpp
Partida _partida{1};
int _eliminadosRegistrados = 0;
```

El `1` es el identificador temporal del jugador. La selección de perfiles y
su persistencia todavía no están conectadas.

Al actualizar una partida en curso se informa el tiempo transcurrido, las
nuevas bajas y el nivel alcanzado:

```cpp
_partida.actualizar(segundos);
_partida.registrarEliminados(eliminados - _eliminadosRegistrados);
_eliminadosRegistrados = eliminados;
_partida.registrarNivel(jugador.getNivel());
```

La diferencia evita repetir bajas. Si antes se habían informado 8 y ahora hay
10, se envían solamente 2.

El gestor limpia los muertos antes de que los proyectiles del fotograma
actual apliquen daño. Por eso `actualizarPartida` suma al acumulado del gestor
los enemigos muertos que todavía permanecen en su vector. En el fotograma
siguiente, esos enemigos pasan al acumulado del gestor sin volver a sumarse.
Esto permite incluir la última baja en el resultado sin esperar otro fotograma.

El orden principal es:

1. Actualizar recargas, jugador, escenario y encuentros.
2. Actualizar enemigos y daño por contacto.
3. Mover proyectiles y resolver sus impactos.
4. Recoger recursos.
5. Actualizar estadísticas y comprobar el desenlace.
6. Crear nuevos disparos solamente si la partida continúa.

La actualización normal retorna antes de avanzar el combate cuando hay pausa
o un menú de estación abierto. Esos períodos no incrementan la duración.

## 3. Chatarra recogida frente a saldo disponible

El registro se realiza en el lugar donde se recoge un recurso del suelo:

```cpp
jugador.ganarChatarra(recurso->cantidad);
_partida.registrarChatarra(recurso->cantidad);
```

El saldo del jugador puede disminuir al comprar. La cantidad recogida durante
la partida permanece acumulada. Ejemplo: recoger 20 y gastar 10 deja un saldo
de 10, pero conserva 20 recogidos para el resultado.

Las devoluciones por cancelar una compra utilizan el mecanismo existente del
jugador y no llaman a `registrarChatarra`. Por eso no generan puntos adicionales.
`Juego` consulta el puntaje calculado por `Partida`; no duplica su fórmula.

## 4. Resolver victoria y derrota

`Juego` consulta el estado del jugador y del jefe para informar el desenlace:

```cpp
if (!jugador.estaVivo())
{
    _partida.marcarDerrota();
}
else if (jefeMuerto)
{
    _partida.marcarVictoria();
}
```

La derrota tiene prioridad si ambos mueren en el mismo fotograma. Para reconocer
la muerte inmediata del jefe también se revisan los muertos pendientes de
limpieza; el tipo de enemigo con ID 4 corresponde al jefe en la interfaz actual.

Al terminar se cambia el estado de la aplicación:

```cpp
if (!_partida.estaEnCurso())
{
    _estado = EstadoJuego::Resultado;
    _opcionMenu = 0;
}
```

La actualización normal solo avanza en `Jugando`. Por eso `Resultado` detiene
movimiento, oleadas, daño y disparos. `actualizarPartida` también retorna si
la partida ya terminó, evitando nuevos informes desde ese método.

## 5. Pantalla de resultado y presentación del jefe

`dibujarResultado` usa la vista predeterminada de la ventana, de modo que el
texto queda fijo en pantalla aunque la cámara haya recorrido el mundo.
Muestra victoria o derrota, duración, bajas, chatarra recogida, nivel y puntaje.

```cpp
const std::string resultado =
    _partida.fueVictoria() ? "VICTORIA" : "DERROTA";
```

Enter inicia otra partida y Escape vuelve al menú principal. Volver a elegir
Jugar desde ese menú también crea una partida nueva.

Durante el combate, la interfaz muestra bajas, puntaje y las consultas del
gestor sobre el jefe. El aviso utiliza `jefeEnCamino()` y
`segundosParaElJefe()`; después se muestra si el jefe está en el mapa.
Su figura se distingue con color naranja. Este incremento no agrega una barra
de vida del jefe: la interfaz actual de `Enemigo` no expone su vida.

## 6. Nueva partida frente a reinicio de prueba

`iniciarNuevaPartida` reconstruye los objetos que deben recuperar sus valores
iniciales:

```cpp
cancelarCompraEstacion();
_estacionSeleccionada = nullptr;
jugador = Jugador{};
_inventarioArmas = InventarioArmas{};
_estacionMejoras = EstacionMejoras{};
_encuentros = EncuentroAleatorio(_generador());
_costoCofre = 15;
aplicarBonificacionesArmas();
reiniciarPrueba();
_estado = EstadoJuego::Jugando;
_opcionMenu = 0;
```

La reconstrucción de `Jugador` restablece nivel, experiencia, vida máxima,
chatarra y velocidad sin modificar su módulo. La reconstrucción del inventario
recupera la pistola inicial; aplicar sus bonificaciones elimina los bonos de
la partida anterior. La estación recupera su costo inicial de 10 y se descartan
las ofertas y zonas anteriores. Los cofres vuelven a costar 15.

Se descarta la referencia a la estación antes de reemplazar los encuentros.
Así no queda un puntero apuntando a una zona de la partida anterior.

| Acción | Comportamiento |
|---|---|
| Jugar desde el menú | Partida nueva con valores iniciales. |
| Enter en resultado | Partida nueva con valores iniciales. |
| R durante el juego | Reinicio de prueba: conserva progreso, compras, precios y encuentros usados. |

El reinicio de prueba limpia oleadas, proyectiles, recursos e indicadores,
restablece recargas y comienza otro registro temporal de estadísticas.

## 7. Dos armas automáticas

Antes se disparaba solamente el arma seleccionada. Ahora se recorren ambos
slots del inventario y se omiten los vacíos:

```cpp
void Juego::actualizarDisparoAutomatico()
{
    for (std::size_t slot = 0; slot < 2; ++slot)
    {
        if (_inventarioArmas.obtenerIdSlot(slot) != 0)
        {
            dispararArma(slot);
        }
    }
}
```

`dispararArma` obtiene el arma de ese slot. Cada arma busca el enemigo vivo
visible más cercano dentro de su propio alcance y dispara si terminó su recarga.
Las recargas permanecen en los objetos `Arma`, por lo que una no reinicia a otra.

Ejemplo: pistola y metralleta pueden disparar en la misma actualización, pero
la metralleta vuelve a disparar antes por su mayor cadencia. Si solo una tiene
alcance suficiente, solamente dispara esa arma.

Cada slot conserva su orientación:

```cpp
std::array<float, 2> _angulosArmas{};
```

`obtenerAnclajeArma` separa los apoyos verticalmente cuando hay dos armas:
16 píxeles arriba para el primer slot y 16 abajo para el segundo. Con una sola
arma se utiliza el centro del jugador. El dibujo recorre los mismos dos slots.

Las teclas 1 y 2 ya no alternan el arma de combate. Se conservan los números
para elegir ofertas y confirmar un reemplazo dentro del menú de estación.

## 8. Disparos desde la boca del arma

El origen anterior era el centro del jugador. Ahora el dibujo y la consulta
de la punta utilizan `AspectoArmas::crearFigura`, que configura tamaño,
pivote, posición, giro y reflejo con los mismos valores.

La altura de la boca se expresa como una fracción del alto del recorte visible:

| Arma | Altura de la boca |
|---|---|
| Pistola | 0,21 del alto |
| Metralleta | 0,28 del alto |
| Escopeta | 0,25 del alto |
| Cañón de energía | 0,43 del alto |

Estas medidas corresponden a los PNG actuales. Si se reemplazan las imágenes,
hay que revisar las medidas de su boca.

El origen vertical de la figura queda a la altura del cañón:

```cpp
figura.setOrigin({ancho * PIVOTES[indice], alto * ALTURAS_BOCA[indice]});
```

Así el eje del cañón pasa por el punto de apoyo y conserva la puntería al
girar. Al apuntar a la izquierda se mantiene el reflejo vertical del dibujo.

`obtenerPunta` transforma el punto local de la boca a coordenadas del mundo:

```cpp
const sf::RectangleShape figura = crearFigura(anclaje, angulo, idArma);
const sf::Vector2f tamanio = figura.getSize();
const sf::Vector2f punta = figura.getTransform().transformPoint(
    {tamanio.x, tamanio.y * ALTURAS_BOCA[idArma - 1]});
return {punta.x, punta.y};
```

`transformPoint` incorpora el pivote, la posición, el giro y el reflejo. No se
mantiene una segunda fórmula separada para intentar reproducir el dibujo.

`Juego` usa esa punta como origen de la descarga:

```cpp
const Posicion punta = _aspectoArmas.obtenerPunta(
    obtenerAnclajeArma(slot), _angulosArmas[slot], arma.obtenerId());

arma.intentarDispararRafaga(
    _proyectiles, punta, destino, velocidad, ancho, alto);
```

`destino` se calcula hacia adelante sobre el eje del arma. El método de `Arma`
recibe un origen central y convierte ese punto a la esquina superior izquierda
del rectángulo del proyectil. Por eso el centro de la bala coincide con la boca.
Los perdigones de una descarga nacen en esa boca y después se desvían según
la dispersión del arma.

Cuando falta la textura se conserva el rectángulo de respaldo y se calcula
su punta con la misma transformación.

## 9. Identidad de enemigos en los impactos

Se retiraron de `Juego` el vector paralelo de identificadores, su contador y
el método de sincronización. Los impactos usan la identidad que ya informa
cada enemigo:

```cpp
proyectil.registrarImpacto(enemigo.obtenerIdInstancia())
```

Esto permite que un proyectil perforante recuerde a quién golpeó aunque el
vector de enemigos se compacte o cambie de capacidad. `Juego` consume la
interfaz existente; no modifica cómo el módulo de enemigos asigna identidades.
