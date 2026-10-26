#pragma once
#include <cstdio>

struct RegistroJugador {
    int nivelMaximo;
    int puntajeMaximo;
    int chatarraTotal;
};

class ArchivoJugadores {
private:
    const char* rutaArchivo = "datos/Jugadores.dat";

public:
    bool guardar(const RegistroJugador& reg);
    RegistroJugador leer();
};

