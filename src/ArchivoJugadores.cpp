#include "ArchivoJugadores.h"

#include "CarpetaDatos.h"
#include <limits>

namespace
{
bool registroValido(const RegistroJugador& registro)
{
    return registro.nivelMaximo >= 1 &&
        registro.nivelMaximo <= (std::numeric_limits<int>::max() - 100) / 50 &&
        registro.puntajeMaximo >= 0 && registro.chatarraTotal >= 0;
}
}

bool ArchivoJugadores::guardar(const RegistroJugador& registro)
{
    if (!registroValido(registro))
    {
        return false;
    }
    asegurarCarpetaDatos();
    FILE* archivo = fopen(obtenerRutaDatos("Jugadores.dat").c_str(), "wb");
    if (archivo == nullptr)
    {
        return false;
    }
    const bool escrito = fwrite(&registro, sizeof(RegistroJugador), 1, archivo) == 1;
    const bool cerrado = fclose(archivo) == 0;
    return escrito && cerrado;
}

RegistroJugador ArchivoJugadores::leer()
{
    const RegistroJugador inicial{1, 0, 0};
    FILE* archivo = fopen(obtenerRutaDatos("Jugadores.dat").c_str(), "rb");
    if (archivo == nullptr)
    {
        return inicial;
    }
    RegistroJugador registro{};
    const bool leido = fread(&registro, sizeof(RegistroJugador), 1, archivo) == 1;
    const bool fin = fgetc(archivo) == EOF && !ferror(archivo);
    fclose(archivo);
    return leido && fin && registroValido(registro) ? registro : inicial;
}
