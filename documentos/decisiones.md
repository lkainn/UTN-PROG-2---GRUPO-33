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
  Configura un proyectil cuando el enfriamiento termino y el proyectil anterior
  esta inactivo; no mueve proyectiles ni resuelve colisiones. El primer disparo
  queda disponible de inmediato.
- Disparo automatico: Juego selecciona al enemigo vivo, calcula los centros y
  solicita el disparo. Arma valida el alcance y calcula una direccion
  normalizada hacia el objetivo. El proyectil comienza a moverse en el
  fotograma siguiente a su creacion.
- Enemigos de demostracion: Juego mantiene tres instancias en arreglos fijos,
  selecciona como objetivo al enemigo vivo mas cercano y administra un tiempo
  de reaparicion individual de 3 segundos. El alcance temporal del arma es de
  2210 pixeles para cubrir la diagonal completa de la ventana.
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
- Archivos binarios previstos: Jugadores.dat, Partidas.dat, Enemigos.dat,
  Armas.dat y Mejoras.dat.

## Alcance de la version minima

La primera version abre una ventana, procesa el evento de cierre, permite salir
con Escape y dibuja un rectangulo de prueba. Todavia no contiene jugabilidad.
