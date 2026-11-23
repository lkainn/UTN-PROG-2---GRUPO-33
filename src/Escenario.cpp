#include "Escenario.h"

#include "Configuracion.h"

#include <array>
#include <fstream>
#include <cmath>

Escenario::Escenario()
    : _texturaSuelo{},
      _fondo({static_cast<float>(Configuracion::ANCHO_VENTANA),
              static_cast<float>(Configuracion::ALTO_VENTANA)}),
      _vista(sf::FloatRect({0.0f, 0.0f}, _fondo.getSize())),
      _fondoDisponible(cargarSuelo())
{
    if (_fondoDisponible)
    {
        _texturaSuelo.setSmooth(false);
        _fondo.setTexture(&_texturaSuelo, true);
    }
    else
    {
        _fondo.setFillColor(sf::Color(28, 31, 36));
    }
}

bool Escenario::cargarSuelo()
{
    constexpr std::array<const char*, 2> RUTAS_SUELO{{
        "recursos/entorno/suelo_chatarra_v1.png",
        "../../recursos/entorno/suelo_chatarra_v1.png"
    }};

    for (const char* ruta : RUTAS_SUELO)
    {
        std::ifstream archivo(ruta, std::ios::binary);
        if (archivo.good() && _texturaSuelo.loadFromFile(ruta))
        {
            return true;
        }
    }
    return false;
}

void Escenario::dibujar(sf::RenderTarget& destino) const
{
    const FormaColision area = obtenerAreaVisible();
    const sf::Vector2f tamanio = _fondo.getSize();
    const float inicioX = std::floor(area.x / tamanio.x) * tamanio.x;
    const float inicioY = std::floor(area.y / tamanio.y) * tamanio.y;
    // Copiar la figura es seguro mientras la textura propietaria siga viva.
    sf::RectangleShape baldosa = _fondo;
    for (int fila = 0; fila < 2; ++fila)
    {
        for (int columna = 0; columna < 2; ++columna)
        {
            baldosa.setPosition({inicioX + columna * tamanio.x,
                                 inicioY + fila * tamanio.y});
            destino.draw(baldosa);
        }
    }
}

void Escenario::actualizar(Posicion posicionJugador)
{
    if (std::isfinite(posicionJugador.x) && std::isfinite(posicionJugador.y))
    {
        _vista.setCenter({posicionJugador.x, posicionJugador.y});
    }
}

const sf::View& Escenario::obtenerVista() const
{
    return _vista;
}

FormaColision Escenario::obtenerAreaVisible() const
{
    const sf::Vector2f centro = _vista.getCenter();
    const sf::Vector2f tamanio = _vista.getSize();
    return {centro.x - tamanio.x / 2.0f, centro.y - tamanio.y / 2.0f,
            tamanio.x, tamanio.y};
}

bool Escenario::fondoDisponible() const
{
    return _fondoDisponible;
}
