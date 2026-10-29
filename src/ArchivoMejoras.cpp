#include "ArchivoMejoras.h"

#include "CarpetaDatos.h"
#include <cstring>

namespace
{
std::vector<Mejora> catalogoInicial()
{
    return {{1, "Vida Maxima", 0, 5, "Aumenta tu salud limite"},
            {2, "Velocidad", 0, 3, "Corres mas rapido"}};
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
    FILE* archivo = fopen(obtenerRutaDatos("Mejoras.dat").c_str(), "wb");
    if (archivo == nullptr)
    {
        return false;
    }
    const bool escrito = fwrite(catalogo.data(), sizeof(Mejora), catalogo.size(), archivo) == catalogo.size();
    const bool cerrado = fclose(archivo) == 0;
    return escrito && cerrado;
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
    return catalogo;
}
