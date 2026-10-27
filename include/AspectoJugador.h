#pragma once

#include "TiposCompartidos.h"
#include <SFML/Graphics.hpp>
#include <array>
#include <cstddef>
#include <string>

class AspectoJugador
{
public:
    explicit AspectoJugador(const char* carpeta = "recursos/personajes");
    void actualizar(Posicion posicion);
    void orientarHacia(Posicion origen, Posicion objetivo);
    Posicion obtenerDireccionVista() const;
    void reiniciar(Posicion posicion);
    void dibujar(sf::RenderTarget& destino, Posicion posicion,
                 bool vivo, bool herido = false) const;
    bool texturaDisponible() const;
    bool animacionDisponible() const;
    Posicion obtenerMano(Posicion posicion, std::size_t slot, bool vivo = true) const;
    void dibujarAgarre(sf::RenderTarget& destino, Posicion posicion, std::size_t slot,
                      bool vivo, bool herido = false) const;

private:
    struct ManoAtlas
    {
        Posicion palma{};
        sf::Vector2i recorteDedos{};
    };

    using PoseAtlas = std::array<ManoAtlas, 2>;
    using DireccionesAtlas = std::array<std::array<PoseAtlas, 24>, 8>;

    std::array<sf::Texture, 8> _cuerposAtlas;
    std::array<sf::Texture, 8> _dedosAtlas;
    std::array<DireccionesAtlas, 2> _posesAtlas{};
    sf::Vector2f _origenAtlas{};
    float _escalaAtlas = 1.0f;
    float _largoPasoAtlas = 1.0f;
    int _direccionAtlas = 1;
    bool _usaAtlas = false;
    bool _enMovimiento = false;
    Posicion _ultimoMovimiento{};
    float _avanceUltimo = 0.0f;

    sf::Texture _textura;
    sf::IntRect _recorte{};
    Posicion _posicionAnterior{};
    bool _disponible = false;
    bool _tienePosicion = false;
    bool _miraIzquierda = false;
    float _distanciaCiclo = 0.0f;
    float _amplitudMovimiento = 0.0f;

    bool cargar(const char* carpeta);
    bool cargarAtlas(const char* carpeta);
    bool cargarAtlasDesde(const std::string& carpeta);
    void actualizarAtlas(Posicion posicion);
    int obtenerCuadroAtlas(bool vivo) const;
    int obtenerClipAtlas(bool vivo) const;
    void dibujarAtlas(sf::RenderTarget& destino, Posicion posicion,
                      bool vivo, sf::Color color) const;
    float obtenerPaso(bool vivo) const;
    sf::Transform obtenerTransformacionCuerpo(Posicion posicion, bool vivo) const;
    sf::Transform obtenerTransformacionBrazo(Posicion posicion, std::size_t slot,
                                             bool antebrazo, bool vivo) const;
    void dibujarArticulado(sf::RenderTarget& destino, Posicion posicion,
                          sf::Color color) const;
};
