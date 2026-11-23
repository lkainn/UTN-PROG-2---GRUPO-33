#pragma once

#include "Enemigo.h"
#include "TiposCompartidos.h"

#include <SFML/Graphics.hpp>

#include <array>
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

        // Lado del icono de un enemigo comun, el del jefe y el de los
        // objetos del mundo. No guardan la escala del mundo: son simbolos, y
        // a escala real serian de cuatro pixeles.
        static const float ICONO;
        static const float ICONO_JEFE;
        static const float ICONO_OBJETO;

        // Carga los iconos de las cuatro variantes desde los atlas de
        // animacion. Si alguno falta, esa variante cae en su marca de color.
        Minimapa();

        void dibujar(sf::RenderTarget& destino,
                     Posicion jugador,
                     const std::vector<Enemigo>& enemigos,
                     const std::vector<Posicion>& cofres,
                     const std::vector<Posicion>& estaciones) const;

    private:
        // Un icono por variante, en el orden del enum: indice = id - 1.
        std::array<sf::Texture, 4> _iconos;
        std::array<bool, 4> _iconoCargado;

        // Cofre y estacion, en ese orden.
        std::array<sf::Texture, 2> _objetos;
        std::array<bool, 2> _objetoCargado;

        // Toma el primer cuadro del atlas de una variante como icono.
        bool cargarIcono(int indice, const char* carpeta, int celda);

        // Toma el primer estado del atlas de un objeto del mundo, recortando
        // el margen transparente para que el icono llene su espacio.
        bool cargarObjeto(int indice, const char* ruta);

        // Dibuja el icono de la variante, o false si no esta disponible.
        bool dibujarIcono(sf::RenderTarget& destino, int idEnemigo,
                          sf::Vector2f centro, float lado) const;

        // Igual para cofres y estaciones.
        bool dibujarObjeto(sf::RenderTarget& destino, int indice,
                           sf::Vector2f centro, float lado) const;

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
