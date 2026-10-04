#pragma once

#include "TiposCompartidos.h"
#include <cstddef>

enum class MejoraBasica
{
    Vida,
    Dano,
    Cadencia
};

// Prototipo aislado: adaptar estas opciones al catalogo del responsable de mejoras.
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
    void abrirMenu();
    void cancelarCompra();
    bool confirmarEleccion(std::size_t indice, MejoraBasica& elegida);
    int obtenerNivelDano() const;
    int obtenerNivelCadencia() const;
    static const char* describirOpcion(std::size_t indice);

private:
    Posicion _posicion;
    int _costo;
    bool _menuAbierto;
    int _nivelDano;
    int _nivelCadencia;
};
