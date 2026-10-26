#pragma once
#include <cstdio>
#include <vector>
#include "Jugador.h"

class ArchivoMejoras {
private:
    const char* rutaArchivo = "datos/Mejoras.dat";

public:
    bool guardarCatalogo(const std::vector<Mejora>& catalogo);
    std::vector<Mejora> leerCatalogo();
};

