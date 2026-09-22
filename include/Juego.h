#pragma once

#include "Proyectil.h"

#include <SFML/Graphics.hpp>

class Juego {
public:
    Juego();

    void ejecutar();

private:
    sf::RenderWindow ventana;
    sf::RectangleShape robotPrueba;
    sf::RectangleShape blancoPrueba;
    sf::RectangleShape figuraProyectilPrueba;
    Proyectil proyectilPrueba;
    sf::Clock relojFotogramas;
    bool blancoImpactado = false;

    void procesarEventos();
    void actualizar();
    void dibujar();
    void dispararProyectil();
    void reiniciarPrueba();
};
