#include "AspectoArmas.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <string>

namespace
{
// Altura de la boca en cada silueta recortada, medida sobre los PNG.
constexpr std::array<float, 4> ALTURAS_BOCA{{0.21f, 0.28f, 0.25f, 0.43f}};
constexpr std::array<float, 4> ALTURAS_EMPUNADURA{{0.75f, 0.70f, 0.72f, 0.70f}};
}

AspectoArmas::AspectoArmas(const char* carpeta)
    : _texturas{}, _recortes{}, _disponibles{}
{
    _disponibles[0] = cargar(0, carpeta, "pistola_concepto_v1.png");
    _disponibles[1] = cargar(1, carpeta, "metralleta_concepto_v1.png");
    _disponibles[2] = cargar(2, carpeta, "escopeta_concepto_v1.png");
    _disponibles[3] = cargar(3, carpeta, "canon_energia_concepto_v1.png");
}

bool AspectoArmas::cargar(std::size_t indice, const char* carpeta, const char* nombre)
{
    const std::string ruta = std::string(carpeta) + "/" + nombre;
    const std::array<std::string, 2> rutas{{ruta, "../../" + ruta}};
    sf::Image imagen;
    bool cargada = false;
    for (const std::string& candidata : rutas)
    {
        std::ifstream archivo(candidata, std::ios::binary);
        if (archivo.good() && imagen.loadFromFile(candidata))
        {
            cargada = true;
            break;
        }
    }
    if (!cargada)
    {
        return false;
    }
    const sf::Vector2u tamanio = imagen.getSize();
    unsigned int izquierda = tamanio.x;
    unsigned int arriba = tamanio.y;
    unsigned int derecha = 0;
    unsigned int abajo = 0;
    for (unsigned int y = 0; y < tamanio.y; ++y)
    {
        for (unsigned int x = 0; x < tamanio.x; ++x)
        {
            // Ignorar residuos casi transparentes mantiene el tamano visible del arma.
            if (imagen.getPixel({x, y}).a >= 32)
            {
                izquierda = std::min(izquierda, x);
                arriba = std::min(arriba, y);
                derecha = std::max(derecha, x);
                abajo = std::max(abajo, y);
            }
        }
    }
    if (izquierda == tamanio.x || !_texturas[indice].loadFromImage(imagen))
    {
        return false;
    }
    // Solo se selecciona la region visible; el PNG original se conserva intacto.
    _recortes[indice] = sf::IntRect(
        {static_cast<int>(izquierda), static_cast<int>(arriba)},
        {static_cast<int>(derecha - izquierda + 1), static_cast<int>(abajo - arriba + 1)});
    _texturas[indice].setSmooth(false);
    return true;
}

void AspectoArmas::dibujar(sf::RenderTarget& destino, Posicion centroJugador,
                         float angulo, bool metralleta) const
{
    dibujar(destino, centroJugador, angulo, metralleta ? 2 : 1);
}

void AspectoArmas::dibujar(sf::RenderTarget& destino, Posicion centroJugador,
                         float angulo, int idArma) const
{
    if (!std::isfinite(centroJugador.x) || !std::isfinite(centroJugador.y) ||
        !std::isfinite(angulo))
    {
        return;
    }
    if (idArma < 1 || idArma > 4)
    {
        return;
    }
    destino.draw(crearFigura(centroJugador, angulo, idArma));
}

sf::RectangleShape AspectoArmas::crearFigura(Posicion centroJugador, float angulo, int idArma,
                                            bool enMano, bool reflejada) const
{
    const std::size_t indice = static_cast<std::size_t>(idArma - 1);
    constexpr std::array<float, 4> ANCHOS{{48.0f, 72.0f, 60.0f, 76.0f}};
    constexpr std::array<float, 4> PIVOTES{{0.23f, 0.38f, 0.30f, 0.25f}};
    const float ancho = ANCHOS[indice];
    float alto = idArma == 1 ? 24.0f : 18.0f;
    if (_disponibles[indice])
    {
        alto = ancho * static_cast<float>(_recortes[indice].size.y) /
            static_cast<float>(_recortes[indice].size.x);
    }
    sf::RectangleShape figura({ancho, alto});
    const float alturaPivote = enMano ? ALTURAS_EMPUNADURA[indice] : ALTURAS_BOCA[indice];
    figura.setOrigin({ancho * PIVOTES[indice], alto * alturaPivote});
    const float separacion = enMano ? 0.0f : 8.0f;
    figura.setPosition({centroJugador.x + std::cos(angulo) * separacion,
                        centroJugador.y + std::sin(angulo) * separacion});
    figura.setRotation(sf::radians(angulo));
    if (enMano ? reflejada : std::cos(angulo) < 0.0f)
    {
        figura.setScale({1.0f, -1.0f});
    }
    if (_disponibles[indice])
    {
        figura.setTexture(&_texturas[indice]);
        figura.setTextureRect(_recortes[indice]);
    }
    else
    {
        const std::array<sf::Color, 4> colores{{sf::Color(155, 160, 170),
            sf::Color(160, 95, 50), sf::Color(190, 130, 60), sf::Color(70, 220, 255)}};
        figura.setFillColor(colores[indice]);
    }
    return figura;
}

Posicion AspectoArmas::obtenerPunta(Posicion anclaje, float angulo, int idArma) const
{
    if (idArma < 1 || idArma > 4 || !std::isfinite(angulo) ||
        !std::isfinite(anclaje.x) || !std::isfinite(anclaje.y))
    {
        return anclaje;
    }
    const sf::RectangleShape figura = crearFigura(anclaje, angulo, idArma);
    const sf::Vector2f tamanio = figura.getSize();
    const sf::Vector2f punta = figura.getTransform().transformPoint(
        {tamanio.x, tamanio.y * ALTURAS_BOCA[idArma - 1]});
    return {punta.x, punta.y};
}

void AspectoArmas::dibujarEnMano(sf::RenderTarget& destino, Posicion mano,
                                float angulo, int idArma, bool reflejada) const
{
    if (idArma < 1 || idArma > 4 || !std::isfinite(angulo) ||
        !std::isfinite(mano.x) || !std::isfinite(mano.y))
    {
        return;
    }
    destino.draw(crearFigura(mano, angulo, idArma, true, reflejada));
}

Posicion AspectoArmas::obtenerPuntaEnMano(Posicion mano, float angulo, int idArma, bool reflejada) const
{
    if (idArma < 1 || idArma > 4 || !std::isfinite(angulo) ||
        !std::isfinite(mano.x) || !std::isfinite(mano.y))
    {
        return mano;
    }
    const sf::RectangleShape figura = crearFigura(mano, angulo, idArma, true, reflejada);
    const sf::Vector2f punta = figura.getTransform().transformPoint(
        {figura.getSize().x, figura.getSize().y * ALTURAS_BOCA[idArma - 1]});
    return {punta.x, punta.y};
}

float AspectoArmas::obtenerAnguloDesdeMano(Posicion mano, Posicion objetivo,
                                         int idArma, bool reflejada) const
{
    const float diferenciaX = objetivo.x - mano.x;
    const float diferenciaY = objetivo.y - mano.y;
    const float distancia = std::hypot(diferenciaX, diferenciaY);
    if (idArma < 1 || idArma > 4 || !std::isfinite(distancia))
    {
        return 0.0f;
    }
    const float direccion = std::atan2(diferenciaY, diferenciaX);
    const sf::RectangleShape figura = crearFigura(mano, 0.0f, idArma, true, reflejada);
    const float separacionBoca = figura.getSize().y *
        (ALTURAS_BOCA[idArma - 1] - ALTURAS_EMPUNADURA[idArma - 1]) *
        (reflejada ? -1.0f : 1.0f);
    if (distancia <= std::abs(separacionBoca) + 0.001f)
    {
        return direccion;
    }
    // La componente perpendicular del objetivo debe coincidir con la de la boca.
    return direccion - std::asin(std::clamp(separacionBoca / distancia, -1.0f, 1.0f));
}

bool AspectoArmas::texturaDisponible(bool metralleta) const
{
    return texturaDisponible(metralleta ? 2 : 1);
}

bool AspectoArmas::texturaDisponible(int idArma) const
{
    return idArma >= 1 && idArma <= 4 && _disponibles[idArma - 1];
}
