#include "EstacionMejoras.h"

#include <algorithm>
#include <cmath>
#include <limits>

EstacionMejoras::EstacionMejoras()
    : _posicion{1100.0f, 540.0f}, _costo(10), _menuAbierto(false),
      _ofertas{}, _ofertasGuardadas{}
{
}

Posicion EstacionMejoras::obtenerPosicion() const
{
    return _posicion;
}

void EstacionMejoras::establecerPosicion(Posicion posicion)
{
    if (!_menuAbierto && std::isfinite(posicion.x) && std::isfinite(posicion.y))
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

OfertaArma EstacionMejoras::generarOferta(int idArma, const InventarioArmas& inventario,
                                        std::mt19937& generador) const
{
    const bool permiteEspecial = inventario.puedeMejorarEspecial(idArma);
    const int sorteo = std::uniform_int_distribution<int>(0, permiteEspecial ? 99 : 96)(generador);
    RarezaMejora rareza = RarezaMejora::Legendaria;
    if (sorteo < 60)
    {
        rareza = RarezaMejora::Comun;
    }
    else if (sorteo < 85)
    {
        rareza = RarezaMejora::Rara;
    }
    else if (sorteo < 97)
    {
        rareza = RarezaMejora::Epica;
    }
    const int probabilidadEspecial = rareza == RarezaMejora::Rara ? 15 :
        (rareza == RarezaMejora::Epica ? 50 : 0);
    const bool especial = permiteEspecial && (rareza == RarezaMejora::Legendaria ||
        (probabilidadEspecial > 0 && std::uniform_int_distribution<int>(0, 99)(generador) < probabilidadEspecial));
    return {idArma, rareza, especial};
}

void EstacionMejoras::abrirMenu(const InventarioArmas& inventario, std::mt19937& generador)
{
    if (_menuAbierto)
    {
        return;
    }
    const double zonaX = std::floor(static_cast<double>(_posicion.x) / 1200.0);
    const double zonaY = std::floor(static_cast<double>(_posicion.y) / 1200.0);
    if (zonaX < std::numeric_limits<int>::min() || zonaX > std::numeric_limits<int>::max() ||
        zonaY < std::numeric_limits<int>::min() || zonaY > std::numeric_limits<int>::max())
    {
        return;
    }
    std::array<int, 4> candidatos{};
    std::size_t cantidad = 0;
    for (int idArma = 1; idArma <= 4; ++idArma)
    {
        if (inventario.puedeRecibirArma(idArma))
        {
            candidatos[cantidad++] = idArma;
        }
    }
    if (cantidad == 0)
    {
        return;
    }
    const std::pair<int, int> zona{static_cast<int>(zonaX), static_cast<int>(zonaY)};
    auto entrada = _ofertasGuardadas.find(zona);
    bool necesitaSorteo = entrada == _ofertasGuardadas.end();
    if (!necesitaSorteo)
    {
        for (const OfertaArma& oferta : entrada->second)
        {
            necesitaSorteo = necesitaSorteo || !inventario.puedeAplicarOferta(oferta);
        }
    }
    if (necesitaSorteo)
    {
        std::shuffle(candidatos.begin(), candidatos.begin() + cantidad, generador);
        auto& guardadas = _ofertasGuardadas[zona];
        for (std::size_t indice = 0; indice < guardadas.size(); ++indice)
        {
            if (!inventario.puedeAplicarOferta(guardadas[indice]))
            {
                guardadas[indice] = generarOferta(candidatos[indice % cantidad], inventario, generador);
            }
        }
    }
    _ofertas = _ofertasGuardadas.at(zona);
    _menuAbierto = true;
}

void EstacionMejoras::cancelarCompra()
{
    _menuAbierto = false;
}

bool EstacionMejoras::confirmarEleccion(std::size_t indice, InventarioArmas& inventario,
                                      bool confirmarReemplazo)
{
    if (!_menuAbierto || indice >= _ofertas.size() ||
        !inventario.aplicarOferta(_ofertas[indice], confirmarReemplazo))
    {
        return false;
    }
    const long long siguienteCosto = (static_cast<long long>(_costo) * 8 + 4) / 5;
    _costo = static_cast<int>(std::min(siguienteCosto,
        static_cast<long long>(std::numeric_limits<int>::max())));
    _menuAbierto = false;
    return true;
}

OfertaArma EstacionMejoras::obtenerOferta(std::size_t indice) const
{
    return indice < _ofertas.size() ? _ofertas[indice] : OfertaArma{};
}

std::string EstacionMejoras::describirOpcion(std::size_t indice) const
{
    const OfertaArma oferta = obtenerOferta(indice);
    if (!oferta.esValida())
    {
        return "";
    }
    std::string texto = std::string(InventarioArmas::obtenerNombreArma(oferta.idArma)) +
        " - " + InventarioArmas::obtenerNombreRareza(oferta.rareza) +
        " | +" + std::to_string(oferta.obtenerPorcentajeDano()) + "% dano";
    if (oferta.obtenerPorcentajeCadencia() > 0)
    {
        texto += " | +" + std::to_string(oferta.obtenerPorcentajeCadencia()) + "% cadencia";
    }
    if (oferta.especial)
    {
        texto += oferta.idArma == 4 ? " | +1 enemigo atravesado" :
            (oferta.idArma == 3 ? " | +1 perdigon" : " | +1 proyectil");
    }
    return texto;
}
