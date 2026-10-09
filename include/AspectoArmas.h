#pragma once

#include "TiposCompartidos.h"
#include <SFML/Graphics.hpp>
#include <array>

class AspectoArmas
{
public:
    explicit AspectoArmas(const char* carpeta = "recursos/armas");
    void dibujar(sf::RenderTarget& destino, Posicion centroJugador,
                 float angulo, bool metralleta) const;
    void dibujar(sf::RenderTarget& destino, Posicion centroJugador,
                 float angulo, int idArma) const;
    bool texturaDisponible(bool metralleta) const;
    bool texturaDisponible(int idArma) const;

private:
    std::array<sf::Texture, 4> _texturas;
    std::array<sf::IntRect, 4> _recortes;
    std::array<bool, 4> _disponibles;
    bool cargar(std::size_t indice, const char* carpeta, const char* nombre);
};
