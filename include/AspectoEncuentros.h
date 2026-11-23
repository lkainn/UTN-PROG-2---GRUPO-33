#pragma once

#include "EncuentroAleatorio.h"

#include <SFML/Graphics.hpp>
#include <array>
#include <cstddef>

class AspectoEncuentros
{
public:
    explicit AspectoEncuentros(const char* carpeta = "recursos/encuentros");
    bool texturaDisponible(TipoEncuentro tipo) const;
    void dibujar(sf::RenderTarget& destino, const ObjetoEncuentro& objeto,
                 bool cerca = false, bool puedeInteractuar = false) const;

private:
    std::array<sf::Texture, 2> _texturas;
    std::array<sf::IntRect, 2> _recortes;
    std::array<int, 2> _anchosCelda;
    std::array<std::array<sf::Vector2f, 2>, 2> _puntosApoyo;
    std::array<bool, 2> _disponibles;

    bool cargar(std::size_t indice, const char* carpeta, const char* nombre);
    void dibujarRespaldo(sf::RenderTarget& destino,
                        const ObjetoEncuentro& objeto) const;
};
