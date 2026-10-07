#pragma once

#include "InventarioArmas.h"
#include "TiposCompartidos.h"

#include <array>
#include <cstddef>
#include <map>
#include <random>
#include <string>
#include <utility>

// Prototipo de ofertas de combate; integrar luego con el catalogo de mejoras.
class EstacionMejoras
{
public:
    EstacionMejoras();
    Posicion obtenerPosicion() const;
    void establecerPosicion(Posicion posicion);
    int obtenerCosto() const;
    bool estaCerca(Posicion posicion) const;
    bool puedeComprar(Posicion posicion, int chatarra) const;
    bool menuAbierto() const;
    void abrirMenu(const InventarioArmas& inventario, std::mt19937& generador);
    void cancelarCompra();
    bool confirmarEleccion(std::size_t indice, InventarioArmas& inventario,
                           bool confirmarReemplazo = false);
    OfertaArma obtenerOferta(std::size_t indice) const;
    std::string describirOpcion(std::size_t indice) const;

private:
    Posicion _posicion;
    int _costo;
    bool _menuAbierto;
    std::array<OfertaArma, 3> _ofertas;
    // Hay una estacion por zona; cancelar conserva sus ofertas durante la ejecucion.
    std::map<std::pair<int, int>, std::array<OfertaArma, 3>> _ofertasGuardadas;
    OfertaArma generarOferta(int idArma, const InventarioArmas& inventario,
                            std::mt19937& generador) const;
};
