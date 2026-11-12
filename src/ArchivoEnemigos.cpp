#include "ArchivoEnemigos.h"

#include "CarpetaDatos.h"

#include <cerrno>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <filesystem>
#include <process.h>
#include <stdexcept>
#include <type_traits>

const char* ArchivoEnemigos::RUTA = "datos/Enemigos.dat";

namespace {

constexpr int CANTIDAD_VARIANTES = 4;
constexpr std::size_t TAMANIO_REGISTRO_ANTERIOR = offsetof(RegistroEnemigo, eliminado);
static_assert(std::is_trivially_copyable_v<RegistroEnemigo>);
static_assert(TAMANIO_REGISTRO_ANTERIOR == 64 && sizeof(RegistroEnemigo) == 68);

// Un registro leido puede venir de un archivo editado a mano o cortado por la
// mitad. Antes de usarlo se revisa que los valores tengan sentido.
bool registroEsUsable(const RegistroEnemigo& registro) {
    return registro.idEnemigo >= 1 && registro.idEnemigo <= CANTIDAD_VARIANTES &&
           registro.eliminado <= 1 &&
           (registro.eliminado == 0 ||
               (registro.idEnemigo != BASICO + 1 && registro.idEnemigo != JEFE + 1)) &&
           registro.vida > 0 && registro.dano >= 0 &&
           std::isfinite(registro.velocidad) && registro.velocidad > 0.0f &&
           registro.experienciaOtorgada >= 0 &&
           registro.chatarraOtorgada >= 0 &&
           std::isfinite(registro.ancho) && std::isfinite(registro.alto) &&
           registro.ancho > 0.0f && registro.alto > 0.0f &&
           registro.nombre[0] != '\0' &&
           std::memchr(registro.nombre, '\0', sizeof(registro.nombre)) != nullptr;
}

bool ordenarCatalogo(const std::vector<RegistroEnemigo>& registros,
                     std::vector<RegistroEnemigo>& ordenados)
{
    if (registros.size() != CANTIDAD_VARIANTES)
    {
        return false;
    }

    for (const RegistroEnemigo& registro : registros)
    {
        if (!registroEsUsable(registro))
        {
            return false;
        }
        RegistroEnemigo& destino = ordenados[registro.idEnemigo - 1];
        if (destino.idEnemigo != 0)
        {
            return false;
        }
        destino = registro;
    }
    return true;
}

}  // namespace

bool ArchivoEnemigos::guardarCatalogo(
        const std::vector<RegistroEnemigo>& catalogo)
{
    std::vector<RegistroEnemigo> ordenados(CANTIDAD_VARIANTES);
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

    const bool escrito = fwrite(ordenados.data(), sizeof(RegistroEnemigo),
        ordenados.size(), archivo) == ordenados.size();
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

std::vector<RegistroEnemigo> ArchivoEnemigos::leerCatalogo()
{
    const std::string ruta = obtenerRutaDatos(RUTA);
    FILE* archivo = fopen(ruta.c_str(), "rb");
    if (archivo == nullptr)
    {
        const char* motivo = errno == ENOENT ? "Falta el catalogo obligatorio " : "No se pudo abrir ";
        throw std::runtime_error(motivo + ruta);
    }

    const long bytes = fseek(archivo, 0, SEEK_END) == 0 ? ftell(archivo) : -1;
    const bool formatoAnterior =
        bytes == static_cast<long>(TAMANIO_REGISTRO_ANTERIOR * CANTIDAD_VARIANTES);
    const bool formatoActual =
        bytes == static_cast<long>(sizeof(RegistroEnemigo) * CANTIDAD_VARIANTES);
    std::vector<RegistroEnemigo> registros(CANTIDAD_VARIANTES);
    bool leido = (formatoAnterior || formatoActual) && fseek(archivo, 0, SEEK_SET) == 0;
    if (leido)
    {
        // El formato anterior coincide con el registro actual hasta el campo eliminado.
        const std::size_t tamanio = formatoAnterior ? TAMANIO_REGISTRO_ANTERIOR : sizeof(RegistroEnemigo);
        for (RegistroEnemigo& registro : registros)
        {
            if (fread(&registro, tamanio, 1, archivo) != 1)
            {
                leido = false;
                break;
            }
        }
    }
    const bool cerrado = fclose(archivo) == 0;
    std::vector<RegistroEnemigo> ordenados(CANTIDAD_VARIANTES);
    if (leido && cerrado && ordenarCatalogo(registros, ordenados))
    {
        return ordenados;
    }
    throw std::runtime_error("El catalogo " + ruta + " esta danado o contiene datos invalidos");
}

Enemigo crearEnemigoDesdeCatalogo(const RegistroEnemigo& registro,
                                  float x, float y) {
    return Enemigo(registro.idEnemigo, x, y, registro.ancho, registro.alto,
                   registro.velocidad, registro.vida, registro.dano,
                   registro.nombre, registro.experienciaOtorgada,
                   registro.chatarraOtorgada);
}

Enemigo crearEnemigo(TipoEnemigo tipo, float x, float y)
{
    const auto catalogo = ArchivoEnemigos{}.leerCatalogo();
    const int indice = tipo >= BASICO && tipo <= JEFE ? tipo : BASICO;
    return crearEnemigoDesdeCatalogo(catalogo[indice], x, y);
}
