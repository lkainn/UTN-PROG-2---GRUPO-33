#pragma once

#include <SFML/Graphics.hpp>
#include "TiposCompartidos.h"

class Escenario
{
public:
    Escenario();
    // La figura conserva un puntero a la textura propia; no debe copiarse.
    Escenario(const Escenario&) = delete;
    Escenario& operator=(const Escenario&) = delete;

    void dibujar(sf::RenderTarget& destino) const;
    void actualizar(Posicion posicionJugador);
    const sf::View& obtenerVista() const;
    FormaColision obtenerAreaVisible() const;
    bool fondoDisponible() const;

private:
    sf::Texture _texturaSuelo;
    sf::RectangleShape _fondo;
    sf::View _vista;
    bool _fondoDisponible;

    bool cargarSuelo();
};
