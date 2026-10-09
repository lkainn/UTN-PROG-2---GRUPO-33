# Estaciones y cofres por zonas

Reemplaza el prototipo con temporizadores. Actualizado con
[ofertas de armas por rareza](guia_adquisicion_rareza_armas.md).

## Distribucion y persistencia

El mundo se divide en zonas de 1200 pixeles. Cada zona tiene una ubicacion posible de estacion (15% de aparicion) y dos ubicaciones posibles de cofres.
Cada ubicacion tiene un 25% de probabilidad de contener un cofre; puede haber
cero, uno o dos. Esto reduce un 75% la cantidad media frente a los dos cofres
garantizados anteriores. Se ubican aleatoriamente en regiones separadas. Se generan las
25 zonas alrededor del jugador con anticipacion (radio de dos zonas), antes
 de que sus objetos entren en la vista. No aparecen ni desaparecen por tiempo.

Cada zona se genera una sola vez. Su semilla depende de la semilla del mapa
 y sus coordenadas: el orden de exploracion no cambia las posiciones. Las
coordenadas negativas usan floor para identificar correctamente las zonas.
Los objetos conservan posicion y estado al alejarse y volver.

La distribucion actual evita amontonamientos mediante margenes y regiones
separadas. Todavia no se vincula a pilas concretas del fondo: el suelo actual
es una textura, sin un catalogo de estructuras para anclar objetos.

## Uso y precios

F interactua dentro de 100 pixeles con el objeto disponible mas cercano.

- Estaciones: una sola compra. El menu muestra tres ofertas de arma con rareza y
  pausa el combate. Confirmar consume esa estacion; queda visible con la
  pantalla apagada. El siguiente precio aumenta un 60%, redondeado hacia
  arriba: 10, 16, 26, 42, 68...
- Cofres: una sola apertura y recompensa automatica, sin menu de eleccion.
  Quedan abiertos y vacios. Su contador de precios es independiente:
  15, 24, 39, 63, 101... El aumento del 60% ocurre tras cobrar y entregar
  la recompensa. Los cofres sin abrir muestran el precio global actual.
- Saldo insuficiente, distancia excesiva, muerte o ausencia de recompensa
  elegible no consumen el objeto ni incrementan su precio.
- R devuelve una compra de estacion pendiente, sin consumirla ni aumentar
  su precio. Conserva objetos usados, zonas generadas, precios, bonos y armas
  adquiridas junto con la progresion del reinicio de prueba existente.
- Los precios calculan en long long y saturan en INT_MAX para evitar overflow.

## Recompensas de cofres

Los cofres entregan blindaje (+20 vida maxima y actual) o experiencia (+40),
con igual probabilidad cuando ambos beneficios son elegibles. No adquieren
armas ni aumentan dano o cadencia. La adquisicion de armas se traslado a
estaciones. Si no puede aumentarse la vida maxima sin desbordar enteros,
no se cobra; si experiencia no es elegible se entrega blindaje.

El resultado se muestra cinco segundos en pantalla. Las estaciones usan el
inventario de dos slots y ofrecen mejoras de dano/cadencia y especiales con
rareza. Proyectiles extra y mayor perforacion de energia ya estan implementados
con topes; rebotes y encadenamiento siguen pendientes. Las reglas anteriores
de mejoras grandes se sustituyen por el balance de rarezas acordado.

## Implementacion y verificacion

EncuentroAleatorio.h/.cpp conserva el nombre del modulo previo pero ahora
administra zonas y objetos persistentes. Juego administra interacciones,
precios, recompensas y dibujo. EstacionMejoras administra ofertas, rareza y
precio; InventarioArmas administra slots y bonos por arma. No se modificaron
Jugador ni Enemigo en esta etapa.

Las zonas se guardan en std::map y contienen arrays fijos de objetos; el mapa
libera automaticamente la memoria al destruirse. El puntero de la estacion
con menu abierto es prestado y permanece valido al insertar otras zonas.
No se borran zonas durante la prueba. La memoria crece con la exploracion;
esta persistencia es en memoria durante la ejecucion, no un guardado en disco.
Solo se dibujan objetos cercanos a la vista. Los dibujos siguen siendo figuras
provisionales de SFML, con estados apagado y abierto visibles.

Pruebas: probar_encuentros.bat comprueba generacion unica, independencia del
orden, separacion, coordenadas negativas, persistencia del estado y puntero,
radio de interaccion, cobro unico, precios independientes y redondeo,
cancelacion/reembolso, muerte, catalogo agotado y saturacion. Tambien pasan
las pruebas de estaciones, proyectiles de Juego y camara y la compilacion Debug.

Correccion de posicion: el dibujo de cada estacion ahora usa
su posicion real del mapa. La prueba captura su pantalla en esa coordenada y
comprueba la interaccion cercana. Un muestreo de 2500 zonas verifica el 25%
de cofres por ubicacion; los slots ausentes no se dibujan ni permiten comprar.

Ajuste de densidad: estaciones al 15% por zona, un 85% menos en promedio
que la estacion garantizada anterior. Se sortea despues de los cofres para
conservar sus posiciones y apariciones con la misma semilla. Las pruebas
comprueban ambas frecuencias y excluyen estaciones ausentes de la interaccion.
