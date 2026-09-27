#pragma once

#include "Arma.h"
#include "Enemigo.h"
#include "Jugador.h"

#include <SFML/Graphics.hpp>

#include <array>
#include <cstddef>

class Juego {
public:
    Juego();

    void ejecutar();

private:
    static constexpr std::size_t CANTIDAD_ENEMIGOS = 3;

    sf::RenderWindow ventana;
    Jugador jugador;
    std::array<Enemigo, CANTIDAD_ENEMIGOS> enemigosPrueba;
    std::array<sf::RectangleShape, CANTIDAD_ENEMIGOS> figurasEnemigosPrueba;
    std::array<float, CANTIDAD_ENEMIGOS> tiemposReaparicion;
    sf::RectangleShape figuraProyectilPrueba;
    Proyectil proyectilPrueba;
    Arma armaPrueba;
    sf::Clock relojFotogramas;

    void procesarEventos();
    void actualizar();
    void actualizarEnemigos(float segundos);
    void actualizarProyectil(float segundos);
    void dibujar();
    void actualizarDisparoAutomatico();
    void reiniciarPrueba();
};
