#include "AspectoProyectiles.h"

#include <algorithm>
#include <fstream>
#include <string>

AspectoProyectiles::AspectoProyectiles(const char* carpeta)
    : _texturas{}, _recortes{}, _disponibles{}
{
    _disponibles[0] = cargar(0, carpeta, "pistola_v1.png");
    _disponibles[1] = cargar(1, carpeta, "metralleta_v1.png");
}

bool AspectoProyectiles::cargar(std::size_t indice, const char* carpeta, const char* nombre)
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
            // Ignorar residuos casi transparentes mantiene el tamano visible de la bala.
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

void AspectoProyectiles::dibujar(sf::RenderTarget& destino, const Proyectil& proyectil) const
{
    if (!proyectil.estaActivo())
    {
        return;
    }
    const FormaColision forma = proyectil.obtenerFormaColision();
    const bool metralleta = forma.ancho == 14.0f && forma.alto == 6.0f;
    const std::size_t indice = metralleta ? 1 : 0;
    sf::RectangleShape figura({forma.ancho, forma.alto});
    figura.setOrigin({forma.ancho / 2.0f, forma.alto / 2.0f});
    figura.setPosition({forma.x + forma.ancho / 2.0f, forma.y + forma.alto / 2.0f});
    figura.setRotation(sf::radians(proyectil.obtenerAngulo()));
    if (_disponibles[indice])
    {
        figura.setTexture(&_texturas[indice]);
        figura.setTextureRect(_recortes[indice]);
    }
    else
    {
        figura.setFillColor(metralleta ? sf::Color(255, 140, 50) : sf::Color(255, 225, 70));
    }
    destino.draw(figura);
}

bool AspectoProyectiles::texturaDisponible(bool metralleta) const
{
    return _disponibles[metralleta ? 1 : 0];
}
