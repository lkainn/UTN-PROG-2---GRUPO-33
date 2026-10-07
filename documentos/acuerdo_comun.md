# Acuerdo comun de integracion

Este acuerdo permite desarrollar los modulos de jugador, enemigos y combate en ramas separadas. La base compartida esta en `include/TiposCompartidos.h` y forma parte del proyecto de Code::Blocks.

## Datos compartidos

| Tipo | Significado |
| --- | --- |
| `Posicion` | Coordenadas `x` e `y` en el espacio de juego. |
| `FormaColision` | Rectangulo con posicion `x`, `y` y dimensiones `ancho`, `alto`. |
| `Recompensa` | Experiencia y chatarra otorgadas al eliminar un enemigo. |

Estas estructuras transportan datos; no reemplazan a las clases `Jugador`, `Enemigo` ni `Proyectil`. Los sistemas no deben leer atributos privados de otras clases para obtener esta informacion. `Posicion` usa las coordenadas del juego en pixeles. En `FormaColision`, `x` e `y` indican la esquina superior izquierda y `ancho` y `alto` son dimensiones no negativas en pixeles. Cada modulo convierte su representacion grafica a este rectangulo al informar una colision.

## Capacidades minimas para integrar

- Jugador: informar posicion y forma de colision; recibir dano y permitir consultar si sigue vivo.
- Enemigo: informar posicion y forma de colision; recibir dano, permitir consultar si sigue vivo y entregar su recompensa una sola vez al morir.
- Proyectil: informar forma de colision y dano; desactivarse tras el primer impacto para evitar impactos repetidos.

Los nombres y firmas concretas de esos metodos se acordaran al crear las clases. Si una interfaz compartida necesita cambiar, comunicarlo a los responsables de los otros modulos antes de modificarla.

## Primer incremento de colisiones

`SistemaColisiones` compara rectangulos sin depender de SFML ni de las clases de entidades. `hayInterseccion` sirve para jugador-enemigo, proyectil-enemigo y jugador-recolectable. Los bordes que solo se tocan no cuentan como impacto. Una forma con ancho o alto menor o igual a cero, o con coordenadas no finitas, no produce colision.

`estaDentroDe` permite comprobar los limites del escenario. Devuelve verdadero si la forma completa esta dentro, incluso cuando sus bordes coinciden con los del escenario. La clase que controla al jugador sera responsable de corregir su posicion si sale de esos limites.

El sistema informa si hay contacto; no aplica dano, no entrega recompensas ni desactiva proyectiles. Esas acciones pertenecen a las entidades y al ciclo de juego, que se integraran cuando existan sus clases.

Para comprobar este modulo sin iniciar la ventana del juego, ejecutar `pruebas/probar_colisiones.bat` desde la carpeta del proyecto.

## Responsabilidades y orden de integracion

- Jugador y progresion: movimiento, limites, vida, experiencia, mejoras, perfil, `Jugadores.dat` y `Mejoras.dat`.
- Enemigos y partida: persecucion, oleadas, jefe, resultado, `Enemigos.dat` y `Partidas.dat`.
- Combate e integracion: armas, proyectiles, colisiones, ciclo de `Juego` y `Armas.dat`.

Cada responsable desarrolla y prueba su modulo en una rama creada desde `desarrollo`. El equipo revisa el aporte antes de incorporarlo a `desarrollo`; luego se hacen pruebas integradas. `main` queda para versiones estables. El primer objetivo integrado es movimiento, un enemigo, un disparo automatico y una colision valida en una misma ejecucion.

El estado temporal de la partida queda en memoria. Los cinco archivos binarios guardan registros simples, no objetos de juego completos.
