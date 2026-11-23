#pragma once

#include "Partida.h"

#include <cstdio>
#include <vector>

// Un renglon de Partidas.dat: el resultado de una partida terminada.
// Son los campos previstos en el informe, en el mismo orden.
//
// A diferencia de los otros catalogos, este archivo CRECE: cada partida suma
// un registro al final y ninguno se pisa.
struct RegistroPartida {
    int idPartida;
    int idJugador;
    int puntaje;
    int nivelAlcanzado;
    int enemigosEliminados;
    int chatarraObtenida;
    int duracionSegundos;
    bool victoria;
};

// Historial de partidas jugadas, en datos/Partidas.dat.
class ArchivoPartidas {
    public:
        static const char* RUTA;

        // Agrega el resultado al final del historial y le asigna su numero de
        // registro a la partida. Solo guarda partidas terminadas: una en curso
        // todavia no tiene duracion ni resultado definitivos.
        bool agregar(Partida& partida);

        std::vector<RegistroPartida> leerTodas();

        // Cuantas partidas hay guardadas. No carga el archivo entero: mira
        // cuanto ocupa y lo divide por el tamanio de un registro.
        int cantidadRegistros();

        // El mejor puntaje del historial, o 0 si no hay ninguna partida.
        // Sirve para el puntajeMaximo del perfil en Jugadores.dat.
        int mejorPuntaje();

        // Borra el historial. Para la opcion de empezar de cero.
        bool vaciar();
};
