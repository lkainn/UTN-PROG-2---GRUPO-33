#pragma once
#include <cstdio>
#include <vector>
#include "Jugador.h"

class ArchivoMejoras {
public:
    bool guardarCatalogo(const std::vector<Mejora>& catalogo);
    std::vector<Mejora> leerCatalogo();
};
