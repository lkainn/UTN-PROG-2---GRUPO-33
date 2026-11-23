#pragma once

#include "Enemigo.h"
#include "TiposCompartidos.h"

#include <SFML/Graphics.hpp>
#include <array>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

class AspectoEnemigos
{
public:
    explicit AspectoEnemigos(const char* carpeta = "recursos/enemigos/mordaza_atlas",
                            const char* carpetaZancudo = "recursos/enemigos/zancudo_atlas",
                            const char* carpetaYunque = "recursos/enemigos/yunque_atlas",
                            const char* carpetaDesguazador = "recursos/enemigos/desguazador_atlas");
    void actualizar(const std::vector<Enemigo>& enemigos,
                    FormaColision jugador, float segundos);
    void reiniciar();
    bool dibujar(sf::RenderTarget& destino, const Enemigo& enemigo) const;
    bool animacionDisponible(int idEnemigo = 1) const;

private:
    struct EstadoVisual
    {
        Posicion anterior{};
        float fase = 0.0f;
        float tiempoContacto = 0.0f;
        int direccion = 1;
        bool enMovimiento = false;
        bool enContacto = false;
        bool presente = false;
    };

    struct PaqueteAtlas
    {
        std::array<sf::Texture, 8> texturas;
        int celda = 192;
        sf::Vector2f origen{};
        float escala = 1.0f;
        float zancada = 64.0f;
        float proyeccion = 1.0f;
        bool disponible = false;
    };

    std::array<PaqueteAtlas, 4> _paquetes;
    std::unordered_map<std::uint64_t, EstadoVisual> _estados;

    bool cargarDesde(int indice, const std::string& carpeta, const char* formatoEsperado);
    int obtenerDireccion(float x, float y, int anterior) const;
    int obtenerCuadro(const EstadoVisual& estado) const;
};
