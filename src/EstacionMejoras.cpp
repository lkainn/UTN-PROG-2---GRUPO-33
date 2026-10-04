#include "EstacionMejoras.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace
{
constexpr std::array<const char*, 3> OPCIONES{{
    "Blindaje: +20 vida maxima y actual",
    "Potencia: +20% del dano base de ambas armas",
    "Cadencia: +15% velocidad base de ataque de ambas armas"
}};
}

EstacionMejoras::EstacionMejoras()
    : _posicion{1100.0f, 540.0f}, _costo(10), _menuAbierto(false),
      _nivelDano(0), _nivelCadencia(0)
{
}

Posicion EstacionMejoras::obtenerPosicion() const
{
    return _posicion;
}

void EstacionMejoras::establecerPosicion(Posicion posicion)
{
    if (std::isfinite(posicion.x) && std::isfinite(posicion.y))
    {
        _posicion = posicion;
    }
}

int EstacionMejoras::obtenerCosto() const
{
    return _costo;
}

bool EstacionMejoras::estaCerca(Posicion posicion) const
{
    return std::isfinite(posicion.x) && std::isfinite(posicion.y) &&
        std::hypot(posicion.x - _posicion.x, posicion.y - _posicion.y) <= 100.0f;
}

bool EstacionMejoras::puedeComprar(Posicion posicion, int chatarra) const
{
    return !_menuAbierto && estaCerca(posicion) && chatarra >= _costo;
}

bool EstacionMejoras::menuAbierto() const
{
    return _menuAbierto;
}

void EstacionMejoras::abrirMenu()
{
    _menuAbierto = true;
}

void EstacionMejoras::cancelarCompra()
{
    _menuAbierto = false;
}

bool EstacionMejoras::confirmarEleccion(std::size_t indice, MejoraBasica& elegida)
{
    if (!_menuAbierto || indice >= OPCIONES.size())
    {
        return false;
    }
    elegida = static_cast<MejoraBasica>(indice);
    if ((elegida == MejoraBasica::Dano && _nivelDano == std::numeric_limits<int>::max()) ||
        (elegida == MejoraBasica::Cadencia && _nivelCadencia == std::numeric_limits<int>::max()))
    {
        return false;
    }
    if (elegida == MejoraBasica::Dano)
    {
        ++_nivelDano;
    }
    else if (elegida == MejoraBasica::Cadencia)
    {
        ++_nivelCadencia;
    }
    const long long siguienteCosto = (static_cast<long long>(_costo) * 8 + 4) / 5;
    _costo = static_cast<int>(std::min(siguienteCosto,
        static_cast<long long>(std::numeric_limits<int>::max())));
    _menuAbierto = false;
    return true;
}

int EstacionMejoras::obtenerNivelDano() const
{
    return _nivelDano;
}

int EstacionMejoras::obtenerNivelCadencia() const
{
    return _nivelCadencia;
}

const char* EstacionMejoras::describirOpcion(std::size_t indice)
{
    return indice < OPCIONES.size() ? OPCIONES[indice] : "";
}
