#include "ArchivoPartidas.h"

#include "CarpetaDatos.h"
#include <cstddef>

const char* ArchivoPartidas::RUTA = "datos/Partidas.dat";

namespace
{
bool registroValido(const RegistroPartida& registro)
{
    const auto* bytes = reinterpret_cast<const unsigned char*>(&registro);
    return bytes[offsetof(RegistroPartida, victoria)] <= 1 &&
        registro.idPartida > 0 && registro.idJugador > 0 && registro.puntaje >= 0 &&
        registro.nivelAlcanzado >= 1 && registro.enemigosEliminados >= 0 &&
        registro.chatarraObtenida >= 0 && registro.duracionSegundos >= 0;
}
}

int ArchivoPartidas::cantidadRegistros() {
    FILE* archivo = fopen(obtenerRutaDatos(RUTA).c_str(), "rb");
    if (archivo == NULL) {
        return 0;  // todavia no se jugo ninguna partida
    }

    // En vez de leer todo el historial, se mide el archivo: fseek al final y
    // ftell dice cuantos bytes ocupa.
    if (fseek(archivo, 0, SEEK_END) != 0)
    {
        fclose(archivo);
        return -1;
    }
    const long bytes = ftell(archivo);
    fclose(archivo);

    if (bytes < 0 || bytes % static_cast<long>(sizeof(RegistroPartida)) != 0)
    {
        return -1;
    }
    if (bytes == 0) {
        return 0;
    }
    return (int)(bytes / (long)sizeof(RegistroPartida));
}

bool ArchivoPartidas::agregar(Partida& partida) {
    // Una partida en curso no se guarda: su duracion y su resultado todavia
    // pueden cambiar.
    if (partida.estaEnCurso() || partida.obtenerIdPartida() > 0) {
        return false;
    }

    const int cantidad = cantidadRegistros();
    if (cantidad < 0)
    {
        return false;
    }
    RegistroPartida registro{};
    registro.idPartida = cantidad + 1;
    registro.idJugador = partida.obtenerIdJugador();
    registro.puntaje = partida.obtenerPuntaje();
    registro.nivelAlcanzado = partida.obtenerNivelAlcanzado();
    registro.enemigosEliminados = partida.obtenerEnemigosEliminados();
    registro.chatarraObtenida = partida.obtenerChatarraObtenida();
    registro.duracionSegundos = partida.obtenerDuracionSegundos();
    registro.victoria = partida.fueVictoria();
    if (!registroValido(registro))
    {
        return false;
    }

    // fopen crea el archivo, pero no la carpeta que lo contiene.
    asegurarCarpetaDatos();

    // "ab" agrega al final sin borrar lo que ya habia, y crea el archivo si no
    // existe. Con "wb" cada partida pisaria el historial entero.
    FILE* archivo = fopen(obtenerRutaDatos(RUTA).c_str(), "ab");
    if (archivo == NULL) {
        return false;  // el disco esta protegido o la ruta es invalida
    }

    const bool escrito =
        fwrite(&registro, sizeof(RegistroPartida), 1, archivo) == 1;
    const bool cerrado = fclose(archivo) == 0;

    // El numero de registro se devuelve a la partida recien si quedo guardada.
    if (escrito && cerrado) {
        partida.asignarIdPartida(registro.idPartida);
    }
    return escrito && cerrado;
}

std::vector<RegistroPartida> ArchivoPartidas::leerTodas() {
    std::vector<RegistroPartida> historial;

    FILE* archivo = fopen(obtenerRutaDatos(RUTA).c_str(), "rb");
    if (archivo == NULL) {
        return historial;  // vacio: todavia no hay partidas
    }

    // fread devuelve cuantos registros COMPLETOS leyo. Si el archivo quedo
    // cortado por la mitad, el ultimo pedazo se descarta en lugar de cargar
    // basura.
    RegistroPartida registro;
    while (fread(&registro, sizeof(RegistroPartida), 1, archivo) == 1) {
        if (registroValido(registro))
        {
            historial.push_back(registro);
        }
    }
    fclose(archivo);

    return historial;
}

int ArchivoPartidas::mejorPuntaje() {
    const std::vector<RegistroPartida> historial = leerTodas();

    int mejor = 0;
    for (const RegistroPartida& registro : historial) {
        if (registro.puntaje > mejor) {
            mejor = registro.puntaje;
        }
    }
    return mejor;
}

bool ArchivoPartidas::vaciar() {
    asegurarCarpetaDatos();

    // Abrir en "wb" y cerrar deja el archivo en cero bytes.
    FILE* archivo = fopen(obtenerRutaDatos(RUTA).c_str(), "wb");
    if (archivo == NULL) {
        return false;
    }
    return fclose(archivo) == 0;
}
