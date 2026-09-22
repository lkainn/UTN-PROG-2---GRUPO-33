#pragma once

#include "TiposCompartidos.h"

/*colisiones

`SistemaColisiones` compara rectangulos sin depender de SFML ni de las clases de entidades.
 `hayInterseccion` sirve para jugador-enemigo, proyectil-enemigo y jugador-recolectable.
  Los bordes que solo se tocan no cuentan como impacto. Una forma con ancho o alto menor o igual a cero, o con coordenadas no finitas, no produce colision.

`estaDentroDe` permite comprobar los limites del escenario.
 Devuelve verdadero si la forma completa esta dentro, incluso cuando sus bordes coinciden con los del escenario.
  La clase que controla al jugador sera responsable de corregir su posicion si sale de esos limites.*/

class SistemaColisiones {
public:
    // Dos rectangulos colisionan solo si comparten una superficie con area.
    static bool hayInterseccion(const FormaColision& primera,
                               const FormaColision& segunda);


    static bool estaDentroDe(const FormaColision& forma,
                            const FormaColision& limites);
};
