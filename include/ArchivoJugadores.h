#pragma once
#include <cstdio>

struct RegistroJugador {
    int nivelMaximo;
    int puntajeMaximo;
    int chatarraTotal;
};

class ArchivoJugadores {
public:
    bool guardar(const RegistroJugador& reg);
    RegistroJugador leer();
};
