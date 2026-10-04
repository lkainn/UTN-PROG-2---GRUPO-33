#include "EncuentroAleatorio.h"

#include <cmath>
#include <limits>
#include <random>

namespace
{
constexpr double TAMANIO_ZONA = 1200.0;
constexpr int RADIO_GENERACION = 2;
}

EncuentroAleatorio::EncuentroAleatorio(std::uint32_t semilla)
    : _semilla(semilla), _zonas{}
{
}

void EncuentroAleatorio::generarAlrededor(Posicion posicion)
{
    if (!std::isfinite(posicion.x) || !std::isfinite(posicion.y))
    {
        return;
    }
    const double zonaX = std::floor(posicion.x / TAMANIO_ZONA);
    const double zonaY = std::floor(posicion.y / TAMANIO_ZONA);
    const double limite = std::numeric_limits<int>::max() - RADIO_GENERACION - 1;
    if (zonaX < -limite || zonaX > limite || zonaY < -limite || zonaY > limite)
    {
        return;
    }
    const int centroX = static_cast<int>(zonaX);
    const int centroY = static_cast<int>(zonaY);
    for (int y = centroY - RADIO_GENERACION; y <= centroY + RADIO_GENERACION; ++y)
    {
        for (int x = centroX - RADIO_GENERACION; x <= centroX + RADIO_GENERACION; ++x)
        {
            generarZona(x, y);
        }
    }
}

void EncuentroAleatorio::generarZona(int x, int y)
{
    const auto clave = std::make_pair(x, y);
    if (_zonas.find(clave) != _zonas.end())
    {
        return;
    }
    // Cada zona tiene su propio sorteo: el orden de exploracion no cambia el mapa.
    std::seed_seq semillas{_semilla, static_cast<std::uint32_t>(x),
                          static_cast<std::uint32_t>(y)};
    std::mt19937 generador(semillas);
    const auto posicion = [&](float minimoX, float maximoX, float minimoY, float maximoY)
    {
        return Posicion{
            static_cast<float>(x * TAMANIO_ZONA +
                std::uniform_real_distribution<float>(minimoX, maximoX)(generador)),
            static_cast<float>(y * TAMANIO_ZONA +
                std::uniform_real_distribution<float>(minimoY, maximoY)(generador))};
    };
    // Margenes y regiones separadas evitan objetos amontonados entre zonas.
    Zona zona{{
        {TipoEncuentro::Estacion, posicion(200.0f, 450.0f, 400.0f, 800.0f), false},
        {TipoEncuentro::Cofre, posicion(700.0f, 1000.0f, 200.0f, 400.0f), false},
        {TipoEncuentro::Cofre, posicion(700.0f, 1000.0f, 800.0f, 1000.0f), false}
    }};
    constexpr double PROBABILIDAD_COFRE = 0.25;
    std::bernoulli_distribution sortearCofre(PROBABILIDAD_COFRE);
    zona[1].presente = sortearCofre(generador);
    zona[2].presente = sortearCofre(generador);
    constexpr double PROBABILIDAD_ESTACION = 0.15;
    zona[0].presente = std::bernoulli_distribution(PROBABILIDAD_ESTACION)(generador);
    _zonas.emplace(clave, zona);
}

ObjetoEncuentro* EncuentroAleatorio::buscarCercano(Posicion posicion, TipoEncuentro tipo)
{
    ObjetoEncuentro* encontrado = nullptr;
    float distanciaMinima = 100.0f;
    for (auto& entrada : _zonas)
    {
        for (ObjetoEncuentro& objeto : entrada.second)
        {
            if (!objeto.presente || objeto.usado || objeto.tipo != tipo)
            {
                continue;
            }
            const float distancia = std::hypot(posicion.x - objeto.posicion.x,
                                               posicion.y - objeto.posicion.y);
            if (std::isfinite(distancia) && distancia <= distanciaMinima)
            {
                encontrado = &objeto;
                distanciaMinima = distancia;
            }
        }
    }
    return encontrado;
}

const EncuentroAleatorio::Zonas& EncuentroAleatorio::obtenerZonas() const
{
    return _zonas;
}
