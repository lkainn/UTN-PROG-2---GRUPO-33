#pragma once

#include "Enemigo.h"
#include "Jugador.h"
#include "Proyectil.h"

#include <SFML/Graphics.hpp>

class Juego {
public:
    Juego();

    void ejecutar();

private:
    sf::RenderWindow ventana;
    Jugador jugador;
    Enemigo enemigoPrueba;
    sf::RectangleShape figuraEnemigoPrueba;
    sf::RectangleShape figuraProyectilPrueba;
    Proyectil proyectilPrueba;
    sf::Clock relojFotogramas;

    void procesarEventos();
    void actualizar();
    void dibujar();
    void dispararProyectil();
    void reiniciarPrueba();
};
