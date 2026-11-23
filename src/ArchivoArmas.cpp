#include "ArchivoArmas.h"

#include "CarpetaDatos.h"

#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <process.h>
#include <stdexcept>
#include <type_traits>

namespace
{
constexpr const char* RUTA = "Armas.dat";
static_assert(std::is_trivially_copyable_v<RegistroArma> && sizeof(RegistroArma) == 56);

bool ordenarCatalogo(const ArchivoArmas::Catalogo& registros,
                     ArchivoArmas::Catalogo& ordenados)
{
    for (const RegistroArma& registro : registros)
    {
        if (registro.idArma < 1 || registro.idArma > static_cast<int>(registros.size()) ||
            registro.nombre[0] == '\0' ||
            std::memchr(registro.nombre, '\0', sizeof(registro.nombre)) == nullptr ||
            registro.dano <= 0 || registro.nivelMaximo <= 0 ||
            !std::isfinite(registro.velocidadAtaque) || registro.velocidadAtaque <= 0.0f ||
            !std::isfinite(registro.alcance) || registro.alcance <= 0.0f ||
            registro.eliminado > 1 || (registro.idArma == 1 && registro.eliminado != 0))
        {
            return false;
        }

        RegistroArma& destino = ordenados[registro.idArma - 1];
        if (destino.idArma != 0)
        {
            return false;
        }
        destino = registro;
    }
    return true;
}
}

ArchivoArmas::Catalogo ArchivoArmas::leerCatalogo() const
{
    const std::string ruta = obtenerRutaDatos(RUTA);
    FILE* archivo = fopen(ruta.c_str(), "rb");
    if (archivo == nullptr)
    {
        const char* motivo = errno == ENOENT ? "Falta el catalogo obligatorio " : "No se pudo abrir ";
        throw std::runtime_error(motivo + ruta);
    }

    Catalogo registros{};
    const long bytesEsperados = static_cast<long>(sizeof(RegistroArma) * registros.size());
    const bool leido = fseek(archivo, 0, SEEK_END) == 0 &&
        ftell(archivo) == bytesEsperados &&
        fseek(archivo, 0, SEEK_SET) == 0 &&
        fread(registros.data(), sizeof(RegistroArma), registros.size(), archivo) == registros.size();
    const bool cerrado = fclose(archivo) == 0;

    Catalogo ordenados{};
    if (leido && cerrado && ordenarCatalogo(registros, ordenados))
    {
        return ordenados;
    }
    throw std::runtime_error("El catalogo " + ruta + " esta danado o contiene datos invalidos");
}

bool ArchivoArmas::guardarCatalogo(const Catalogo& catalogo) const
{
    Catalogo ordenados{};
    if (!ordenarCatalogo(catalogo, ordenados))
    {
        return false;
    }

    asegurarCarpetaDatos();
    const std::string destino = obtenerRutaDatos(RUTA);
    const std::string temporal = destino + "." + std::to_string(_getpid()) + ".tmp";
    FILE* archivo = fopen(temporal.c_str(), "wb");
    if (archivo == nullptr)
    {
        return false;
    }

    const bool escrito = fwrite(ordenados.data(), sizeof(RegistroArma), ordenados.size(), archivo)
        == ordenados.size();
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
