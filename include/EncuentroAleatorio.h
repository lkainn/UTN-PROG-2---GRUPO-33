#pragma once

#include "TiposCompartidos.h"
#include <array>
#include <map>
#include <cstdint>
#include <utility>

enum class TipoEncuentro
{
    Estacion,
    Cofre
};

struct ObjetoEncuentro
{
    TipoEncuentro tipo;
    Posicion posicion;
    bool usado;
    bool presente = true;
};

class EncuentroAleatorio
{
public:
    using Zona = std::array<ObjetoEncuentro, 3>;
    using Zonas = std::map<std::pair<int, int>, Zona>;

    explicit EncuentroAleatorio(std::uint32_t semilla);
    void generarAlrededor(Posicion posicion);
    ObjetoEncuentro* buscarCercano(Posicion posicion, TipoEncuentro tipo);
    const Zonas& obtenerZonas() const;

private:
    std::uint32_t _semilla;
    Zonas _zonas;
    void generarZona(int x, int y);
};
