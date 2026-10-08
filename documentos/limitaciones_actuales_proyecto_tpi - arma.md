# Limitaciones actuales del Proyecto TPI

Este documento registra limitaciones de la demostracion actual. No todas son
errores: varias delimitan el alcance de los incrementos ya implementados y
permiten identificar trabajo futuro.

## Arma y proyectiles

- Solo puede existir un proyectil activo a la vez.
- El proyectil no persigue al enemigo despues de ser disparado.
- El alcance se comprueba al crear el disparo, pero el proyectil no almacena ni
  controla una distancia maxima recorrida.
- El nivel del arma se guarda, pero todavia no modifica sus estadisticas.
- Solo existe un arma de prueba.
- Las estadisticas del arma todavia no se cargan desde `Armas.dat`.

## Enemigos

- La cantidad de enemigos esta fijada en tres mediante `std::array`.
- Todos los enemigos usan las mismas estadisticas.
- Las posiciones iniciales y de reaparicion estan escritas en el codigo.
- Los enemigos reaparecen siempre despues de tres segundos.
- Todavia no existen tipos diferentes de enemigos ni un jefe final.
- No existe un sistema completo de oleadas.

## Combate

- La seleccion automatica solamente considera al enemigo vivo mas cercano.
- Un proyectil solo puede danar al primer enemigo impactado.
- La colision de contacto entre jugador y enemigo todavia no aplica dano.
- No hay efectos visuales o sonoros definitivos para disparos, impactos o
  reapariciones.

## Persistencia

- Los enemigos vivos y sus temporizadores existen solamente en memoria.
- Los proyectiles activos y el cooldown del arma tampoco se guardan.
- Los archivos binarios de armas, enemigos, jugadores, partidas y mejoras aun
  no estan integrados al ciclo completo del juego.

## Presentacion y contenido

- Las figuras actuales son rectangulos temporales de SFML.
- Todavia no se integraron sprites ni animaciones definitivas.
- No existe una interfaz final para vida, experiencia, tiempo o nivel del arma.
- La demostracion no representa todavia una partida completa con victoria y
  derrota.
