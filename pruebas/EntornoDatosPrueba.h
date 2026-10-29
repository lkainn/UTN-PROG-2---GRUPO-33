#pragma once

#include <chrono>
#include <cstdlib>
#include <direct.h>
#include <io.h>
#include <process.h>
#include <string>

inline std::string obtenerCarpetaSalidasPrueba()
{
    std::string base = ".";
    if (_access("ProyectoTPI.cbp", 0) == 0)
    {
        _mkdir("bin");
        _mkdir("bin/Debug");
        base = "bin/Debug";
    }
    const std::string carpeta = base + "/salidas_pruebas";
    _mkdir(carpeta.c_str());
    return carpeta;
}

inline bool prepararDatosPrueba()
{
    const char* configurada = std::getenv("TPI_CARPETA_DATOS");
    if (configurada != nullptr && configurada[0] != '\0')
    {
        return true;
    }
    std::string base = ".";
    if (_access("ProyectoTPI.cbp", 0) == 0)
    {
        _mkdir("bin");
        _mkdir("bin/Debug");
        base = "bin/Debug";
    }
    base += "/datos_pruebas";
    _mkdir(base.c_str());
    const auto instante = std::chrono::steady_clock::now().time_since_epoch().count();
    const std::string carpeta = base + "/sesion_" + std::to_string(_getpid()) + "_" + std::to_string(instante);
    if (_mkdir(carpeta.c_str()) != 0)
    {
        return false;
    }
    return _putenv_s("TPI_CARPETA_DATOS", carpeta.c_str()) == 0;
}
