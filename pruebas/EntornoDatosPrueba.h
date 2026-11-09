#pragma once

#include <chrono>
#include <cstdlib>
#include <direct.h>
#include <filesystem>
#include <io.h>
#include <process.h>
#include <string>
#include <stdexcept>

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

inline bool copiarCatalogosPrueba(const std::filesystem::path& destino)
{
    const std::filesystem::path origen = _access("ProyectoTPI.cbp", 0) != 0 &&
        _access("../../ProyectoTPI.cbp", 0) == 0 ? "../../datos" : "datos";
    std::error_code error;
    std::filesystem::create_directories(destino, error);
    if (error || std::filesystem::equivalent(origen, destino, error))
    {
        return false;
    }
    for (const char* nombre : {"Armas.dat", "Enemigos.dat"})
    {
        std::filesystem::copy_file(origen / nombre, destino / nombre,
            std::filesystem::copy_options::skip_existing, error);
        if (error)
        {
            return false;
        }
    }
    return true;
}

inline bool prepararDatosPrueba()
{
    const char* configurada = std::getenv("TPI_CARPETA_DATOS");
    if (configurada != nullptr && configurada[0] != '\0')
    {
        return copiarCatalogosPrueba(configurada);
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
    return copiarCatalogosPrueba(carpeta) && _putenv_s("TPI_CARPETA_DATOS", carpeta.c_str()) == 0;
}

template <typename Archivo>
bool lecturaRechazada(Archivo& archivo, const char* nombre, const char* motivo)
{
    try
    {
        archivo.leerCatalogo();
    }
    catch (const std::runtime_error& error)
    {
        const std::string mensaje = error.what();
        return mensaje.find(nombre) != std::string::npos && mensaje.find(motivo) != std::string::npos;
    }
    return false;
}
