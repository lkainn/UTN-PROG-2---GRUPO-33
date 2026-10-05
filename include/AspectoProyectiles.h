#pragma once

#include "Proyectil.h"
#include <SFML/Graphics.hpp>
#include <array>

class AspectoProyectiles
{
public:
    explicit AspectoProyectiles(const char* carpeta = "recursos/proyectiles");
    void dibujar(sf::RenderTarget& destino, const Proyectil& proyectil) const;
    bool texturaDisponible(bool metralleta) const;

private:
    std::array<sf::Texture, 2> _texturas;
    std::array<sf::IntRect, 2> _recortes;
    std::array<bool, 2> _disponibles;
    bool cargar(std::size_t indice, const char* carpeta, const char* nombre);
};
