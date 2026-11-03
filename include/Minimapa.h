#pragma once

#include "Enemigo.h"
#include "TiposCompartidos.h"

#include <SFML/Graphics.hpp>

#include <vector>

// Radar del sector, en una esquina de la pantalla.
//
// Es un radar y no un mapa del mundo entero porque el mundo no tiene bordes:
// las zonas de encuentros se generan alrededor del jugador a medida que
// avanza. Entonces se muestra un radio fijo con el jugador siempre al centro.
//
// No conoce a Juego ni a los sistemas: recibe posiciones y las dibuja. Los
// cofres y las estaciones llegan como listas de Posicion para no depender de
// los tipos del modulo de encuentros.
class Minimapa {
    public:
        // Lado del recuadro en pantalla, en pixeles.
        static const float LADO;

        // Cuanto mundo abarca, medido desde el jugador hasta el borde. Con
        // 1800 entran holgadamente los enemigos, que aparecen a 700.
        static const float ALCANCE;

        // Separacion respecto de la esquina de la ventana.
        static const float MARGEN;

        Minimapa();

        void dibujar(sf::RenderTarget& destino,
                     Posicion jugador,
                     const std::vector<Enemigo>& enemigos,
                     const std::vector<Posicion>& cofres,
                     const std::vector<Posicion>& estaciones) const;

    private:
        // Pasa una posicion del mundo a coordenadas del recuadro.
        sf::Vector2f aPantalla(Posicion punto, Posicion jugador,
                               sf::Vector2f esquina) const;

        // Que tan lejos esta un punto del jugador, al cuadrado. Se compara
        // asi para no calcular raices cuadradas en cada marca.
        static float distanciaCuadrada(Posicion a, Posicion b);

        // Cada tipo de marca tiene su forma, para que no dependa solo del
        // color: circulo los enemigos, cuadrado los cofres, rombo las
        // estaciones.
        void dibujarMarca(sf::RenderTarget& destino, sf::Vector2f centro,
                          float radio, sf::Color color) const;
        void dibujarCuadrado(sf::RenderTarget& destino, sf::Vector2f centro,
                             float lado, sf::Color color) const;
        void dibujarRombo(sf::RenderTarget& destino, sf::Vector2f centro,
                          float radio, sf::Color color) const;
};
