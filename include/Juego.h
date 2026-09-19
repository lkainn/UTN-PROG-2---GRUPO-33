#ifndef JUEGO_H
#define JUEGO_H

#include <SFML/Graphics.hpp>

class Juego {
public:
    Juego();

    void ejecutar();

private:
    sf::RenderWindow ventana;
    sf::RectangleShape robotPrueba;

    void procesarEventos();
    void actualizar();
    void dibujar();
};

#endif

