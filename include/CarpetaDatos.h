#pragma once

#include <direct.h>
#include <io.h>
#include <cstdlib>
#include <cstring>
#include <string>

// Crea la carpeta "datos" si todavia no existe.
//
// fopen no crea carpetas. Raiz y bin/Debug comparten los mismos archivos;
// TPI_CARPETA_DATOS permite ejecutar pruebas sin tocar el progreso del jugador.
//
// _mkdir devuelve -1 si la carpeta ya existia, y eso no es un problema: lo
// unico que importa es que exista despues de llamarla.
inline std::string obtenerCarpetaDatos()
{
    const char* carpetaPruebas = std::getenv("TPI_CARPETA_DATOS");
    if (carpetaPruebas != nullptr && carpetaPruebas[0] != '\0')
    {
        return carpetaPruebas;
    }
    if (_access("ProyectoTPI.cbp", 0) == 0)
    {
        return "datos";
    }
    if (_access("../../ProyectoTPI.cbp", 0) == 0)
    {
        return "../../datos";
    }
    return "datos";
}

inline std::string obtenerRutaDatos(const char* archivo)
{
    const char* nombre = std::strrchr(archivo, '/');
    return obtenerCarpetaDatos() + "/" + (nombre == nullptr ? archivo : nombre + 1);
}

inline void asegurarCarpetaDatos()
{
    _mkdir(obtenerCarpetaDatos().c_str());
}
