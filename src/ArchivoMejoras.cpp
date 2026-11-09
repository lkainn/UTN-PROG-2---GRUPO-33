#include "ArchivoMejoras.h"

#include "CarpetaDatos.h"
#include <algorithm>
#include <cstring>
#include <filesystem>

namespace
{
constexpr Mejora REBOTE{3, "Rebote", 0, 3, "Tus balas rebotan a otro enemigo"};

std::vector<Mejora> catalogoInicial()
{
    return {{1, "Vida Maxima", 0, 5, "Aumenta tu salud limite"},
            {2, "Velocidad", 0, 3, "Corres mas rapido"}, REBOTE};
}

bool mejoraValida(const Mejora& mejora)
{
    return mejora.idMejora > 0 && mejora.nivelMaximo >= 0 && mejora.nivelMaximo <= 1000 &&
        mejora.nivelActual >= 0 && mejora.nivelActual <= mejora.nivelMaximo &&
        std::memchr(mejora.nombre, '\0', sizeof(mejora.nombre)) != nullptr &&
        std::memchr(mejora.descripcion, '\0', sizeof(mejora.descripcion)) != nullptr;
}
}

bool ArchivoMejoras::guardarCatalogo(const std::vector<Mejora>& catalogo)
{
    if (catalogo.empty())
    {
        return false;
    }
    for (const Mejora& mejora : catalogo)
    {
        if (!mejoraValida(mejora))
        {
            return false;
        }
    }
    asegurarCarpetaDatos();
    const std::string destino = obtenerRutaDatos("Mejoras.dat");
    const std::string temporal = destino + ".tmp";
    FILE* archivo = fopen(temporal.c_str(), "wb");
    if (archivo == nullptr)
    {
        return false;
    }
    const bool escrito = fwrite(catalogo.data(), sizeof(Mejora), catalogo.size(), archivo) == catalogo.size();
    const bool cerrado = fclose(archivo) == 0;
    std::error_code error;
    if (escrito && cerrado)
    {
        std::filesystem::rename(temporal, destino, error);
        if (!error)
        {
            return true;
        }
    }
    std::filesystem::remove(temporal, error);
    return false;
}

std::vector<Mejora> ArchivoMejoras::leerCatalogo()
{
    FILE* archivo = fopen(obtenerRutaDatos("Mejoras.dat").c_str(), "rb");
    std::vector<Mejora> catalogo;
    if (archivo != nullptr)
    {
        Mejora mejora{};
        while (fread(&mejora, sizeof(Mejora), 1, archivo) == 1)
        {
            if (mejoraValida(mejora))
            {
                catalogo.push_back(mejora);
            }
        }
        fclose(archivo);
    }
    if (catalogo.empty())
    {
        catalogo = catalogoInicial();
        guardarCatalogo(catalogo);
    }
    else if (std::none_of(catalogo.begin(), catalogo.end(), [](const Mejora& mejora)
    {
        return mejora.idMejora == REBOTE.idMejora;
    }))
    {
        catalogo.push_back(REBOTE);
    }
    return catalogo;
}
