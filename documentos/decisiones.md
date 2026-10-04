# Decisiones iniciales del Proyecto TPI

- Biblioteca grafica: SFML 3.1.0.
- IDE: Code::Blocks.
- Compilador verificado: MinGW-w64 GCC 14.2.0, UCRT, POSIX, SEH, 64 bits.
- Estandar: C++17.
- Resolucion logica: 1920 x 1080.
- Limite inicial: 60 FPS.
- Victoria: derrotar al jefe final.
- Derrota: vida del jugador igual o menor que cero.
- Colisiones iniciales: rectangulares.
- Proyectil: logica independiente de SFML, con velocidad por ejes en pixeles
  por segundo. La representacion grafica queda a cargo del ciclo de juego.
- Arma: conserva sus estadisticas y controla la cadencia en ataques por segundo.
  Configura un proyectil nuevo e inactivo cuando el enfriamiento termino;
  no mueve proyectiles ni resuelve colisiones. El primer disparo
  queda disponible de inmediato.
- Disparo automatico: Juego selecciona al enemigo vivo, calcula los centros y
  solicita el disparo. Arma valida el alcance y calcula una direccion
  normalizada hacia el objetivo. El proyectil comienza a moverse en el
  fotograma siguiente a su creacion.
- Disparos simultaneos: Juego conserva los proyectiles por valor en un
  `std::vector`, con memoria administrada automaticamente. Cada disparo se
  actualiza e impacta al primer enemigo vivo que intersecta; se elimina al
  impactar o salir de la ventana. La limpieza se realiza despues del recorrido
  para no invalidar referencias. Reiniciar vacia el vector y restablece la
  cadencia; los disparos en vuelo no bloquean nuevos disparos.
- Enemigos de demostracion: Juego mantiene tres instancias en arreglos fijos,
  selecciona como objetivo al enemigo vivo mas cercano y administra un tiempo
  de reaparicion individual de 3 segundos. El alcance inicial de 2210 pixeles
  fue reemplazado por el limite de 600 pixeles descrito mas abajo.
- Dano por contacto: un enemigo que intersecta al jugador aplica su dano y
  activa 0.75 segundos de invulnerabilidad global. Cada impacto genera un
  indicador numerico flotante durante 0.8 segundos. Los indicadores se guardan
  en un `std::vector` porque su cantidad varia y el contenedor administra su
  memoria automaticamente.
- Recursos de enemigos: al morir, cada enemigo genera un cuadrado celeste con
  su experiencia y otro amarillo con su chatarra. Juego conserva estos recursos
  temporales en un `std::vector`; se recogen una sola vez cuando el centro del
  jugador entra en un radio de 90 pixeles y se eliminan inmediatamente del
  vector. La interfaz muestra nivel, experiencia actual y chatarra, y cada
  recoleccion genera un indicador flotante con la cantidad obtenida.
- Clases: flexibles; se confirmaran por incremento.
- Segunda arma de prueba: metralleta, id 2, dano 6, cadencia 8 ataques por
  segundo, alcance de prueba 600 pixeles y nivel maximo 5. Sus proyectiles
  viajan a 800 pixeles por segundo y miden 14 x 6 pixeles, de color naranja.
  La pistola conserva dano 25 y cadencia 2. Cada arma mantiene y actualiza su
  propio enfriamiento. Las teclas 1 y 2 son exclusivamente controles de prueba;
  la adquisicion mediante progreso o mejoras se definira con el equipo.
  R reinicia ambos enfriamientos y selecciona la pistola. Para estos dos
  formatos temporales, la vista distingue el color por dimensiones del disparo,
  sin depender del arma seleccionada ni modificar la logica de Proyectil.
- Archivos binarios previstos: Jugadores.dat, Partidas.dat, Enemigos.dat,
  Armas.dat y Mejoras.dat.
- Estacion de mejoras: modulo `EstacionMejoras.h/.cpp`,
  por valor en Juego, ubicado inicialmente en (1100, 540). Radio de interaccion
  100 pixeles con F. Cobra 10 de chatarra y ofrece tres opciones fijas con
  combate pausado: +20 vida maxima/actual, +20% dano base o +15% cadencia base.
  Dano y cadencia se suman sobre la base y afectan ambas armas, incluso cuando
  no estan equipadas; no modifican balas en vuelo. El dano se redondea hacia
  arriba. Una eleccion confirmada aumenta el precio un 60%, redondeado hacia
  arriba con enteros de 64 bits y saturado al maximo de int. Durante el menu no
  se admiten otras compras ni teclas de prueba; R reembolsa el pago pendiente.
  R conserva mejoras confirmadas y costo, al igual que el nivel y la chatarra
  que ya conserva Jugador. No agrega mejoras automaticas por nivel ni archivos
  binarios nuevos. Grafico provisional con formas de SFML. Los metodos
  gastarChatarra y aumentarVidaMaxima son TEMPORALES autorizados; adaptar al
  responsable de Jugador y a su catalogo definitivo.
- Rango de disparo: pistola y metralleta buscan
  enemigos vivos cuyo centro sea visible y este a no mas de 600 pixeles.
  Cada proyectil de Arma recibe ese alcance como distancia maxima recorrida,
  medida desde su origen y no desde la posicion actual del jugador. Al
  agotarse se desactiva antes de comprobar impactos; salir de la vista tambien
  lo elimina. El alcance sigue siendo un atributo de cada arma, independiente
  del tamano de ventana y de la camara. Proyectiles construidos directamente
  sin un limite explicito conservan el comportamiento anterior.
- Primer entorno visual: deposito de chatarra al aire
  libre. La clase `Escenario` administra la textura del suelo por valor y se
  dibuja antes de las entidades. El fondo completo generado mide 1672 x 941,
  se ajusta a la resolucion logica sin suavizado y no agrega obstaculos.
  Carga desde la raiz o desde `bin/Debug`; si falta la imagen, usa el fondo
  liso anterior. Los futuros objetos se disenaran como sprites separados.
- Camara de seguimiento: Escenario conserva una vista
  de 1920 x 1080 centrada en el jugador y dibuja cuatro copias del suelo
  ancladas al mundo para cubrirla, incluso en coordenadas negativas. No hay
  borde fisico del escenario en este prototipo. Juego usa esa vista para
  entidades, recursos e indicadores y vuelve a la vista fija para la interfaz.
  Los disparos se limpian al abandonar el area visible; los enemigos de prueba
  reaparecen en posiciones relativas a la ubicacion actual del jugador.
  `Jugador::actualizar(bool limitarALaVentana = true)` es una adaptacion
  TEMPORAL autorizada: Juego invoca false y las pruebas del modulo conservan
  el comportamiento original por defecto. Adaptar al responsable de Jugador.

## Alcance de la version minima

La primera version abre una ventana, procesa el evento de cierre, permite salir
con Escape y dibuja un rectangulo de prueba. Todavia no contiene jugabilidad.

## Encuentros aleatorios
Estaciones y cofres aparecen temporalmente alrededor del jugador. F interactua con el mas cercano. Cofres: costo de prueba 15, recompensa automatica por arma, 15% metralleta si no esta adquirida y bonos simples limitados. Se conserva el precio creciente de estacion y las adquisiciones al reiniciar. Detalles y pruebas en documentos/encuentros_aleatorios.md. Ricochet, proyectiles extra y encadenamiento quedan pendientes con desventajas obligatorias.

## Distribucion por zonas
Reemplaza las apariciones temporales: zonas generadas una sola vez, estaciones de una compra que quedan apagadas y cofres abiertos persistentes. Cada tipo aumenta su propio precio 60% tras una compra efectiva; cofres parten de 15. R conserva estados y precios y devuelve la compra pendiente sin consumir. Ver documentos/encuentros_aleatorios.md.

## Correccion de estaciones y densidad de cofres
El dibujo de estaciones usa objeto.posicion, igual que la busqueda de interaccion. Cada slot de cofre tiene un 25% de aparicion (antes 100%); se conserva el mapa persistente. Verificado con captura de render antes de presentar el fotograma, interaccion, muestreo de spawn y regresion de estaciones.

## Texturas de proyectiles
Pistola y metralleta utilizan PNG transparentes generados con imagegen en recursos/proyectiles. AspectoProyectiles carga, selecciona la region visible y orienta los dibujos; respaldo con rectangulos si faltan archivos. Ver recursos/proyectiles/diseno.md para dimensiones, limites y pruebas.
