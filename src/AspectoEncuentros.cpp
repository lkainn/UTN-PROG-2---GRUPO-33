#include "AspectoEncuentros.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <string>

AspectoEncuentros::AspectoEncuentros(const char* carpeta)
    : _texturas{}, _recortes{}, _anchosCelda{}, _puntosApoyo{}, _disponibles{}
{
    _disponibles[0] = cargar(0, carpeta, "estacion_mejoras_v1.png");
    _disponibles[1] = cargar(1, carpeta, "contenedor_suministros_v1.png");
}

bool AspectoEncuentros::cargar(std::size_t indice, const char* carpeta,
                              const char* nombre)
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
    if (tamanio.x < 2 || tamanio.y == 0 || tamanio.x % 2 != 0)
    {
        return false;
    }
    const unsigned int anchoCelda = tamanio.x / 2;
    unsigned int izquierda = anchoCelda;
    unsigned int arriba = tamanio.y;
    unsigned int derecha = 0;
    unsigned int abajo = 0;
    std::array<bool, 2> estadosVisibles{{false, false}};
    std::array<unsigned int, 2> izquierdas{{anchoCelda, anchoCelda}};
    std::array<unsigned int, 2> derechas{{0, 0}};
    std::array<unsigned int, 2> bases{{0, 0}};
    for (unsigned int y = 0; y < tamanio.y; ++y)
    {
        for (unsigned int x = 0; x < tamanio.x; ++x)
        {
            if (imagen.getPixel({x, y}).a >= 32)
            {
                const unsigned int estado = x / anchoCelda;
                const unsigned int localX = x % anchoCelda;
                estadosVisibles[estado] = true;
                izquierdas[estado] = std::min(izquierdas[estado], localX);
                derechas[estado] = std::max(derechas[estado], localX);
                bases[estado] = std::max(bases[estado], y);
                izquierda = std::min(izquierda, localX);
                arriba = std::min(arriba, y);
                derecha = std::max(derecha, localX);
                abajo = std::max(abajo, y);
            }
        }
    }
    if (!estadosVisibles[0] || !estadosVisibles[1] ||
        !_texturas[indice].loadFromImage(imagen))
    {
        return false;
    }
    // Un recorte comun conserva la escala; cada base corrige el margen del dibujo.
    _recortes[indice] = sf::IntRect(
        {static_cast<int>(izquierda), static_cast<int>(arriba)},
        {static_cast<int>(derecha - izquierda + 1),
         static_cast<int>(abajo - arriba + 1)});
    _anchosCelda[indice] = static_cast<int>(anchoCelda);
    for (std::size_t estado = 0; estado < 2; ++estado)
    {
        _puntosApoyo[indice][estado] = {
            (izquierdas[estado] + derechas[estado] + 1) / 2.0f - izquierda,
            static_cast<float>(bases[estado] + 1 - arriba)};
    }
    _texturas[indice].setSmooth(false);
    return true;
}

bool AspectoEncuentros::texturaDisponible(TipoEncuentro tipo) const
{
    return _disponibles[tipo == TipoEncuentro::Estacion ? 0 : 1];
}

void AspectoEncuentros::dibujar(sf::RenderTarget& destino,
                               const ObjetoEncuentro& objeto,
                               bool cerca, bool puedeInteractuar) const
{
    if (!objeto.presente || !std::isfinite(objeto.posicion.x) ||
        !std::isfinite(objeto.posicion.y))
    {
        return;
    }
    const bool estacion = objeto.tipo == TipoEncuentro::Estacion;
    const std::size_t indice = estacion ? 0 : 1;
    if (cerca && !objeto.usado)
    {
        sf::CircleShape indicador(58.0f, 32);
        indicador.setOrigin({58.0f, 58.0f});
        indicador.setScale({1.0f, 0.35f});
        indicador.setPosition({objeto.posicion.x, objeto.posicion.y + 28.0f});
        indicador.setFillColor(sf::Color::Transparent);
        indicador.setOutlineThickness(3.0f);
        indicador.setOutlineColor(puedeInteractuar ?
            (estacion ? sf::Color(85, 235, 165) : sf::Color(255, 200, 70)) :
            sf::Color(155, 155, 155));
        destino.draw(indicador);
    }
    if (!_disponibles[indice])
    {
        dibujarRespaldo(destino, objeto);
        return;
    }
    sf::IntRect recorte = _recortes[indice];
    if (objeto.usado)
    {
        recorte.position.x += _anchosCelda[indice];
    }
    sf::Sprite figura(_texturas[indice], recorte);
    const float ancho = static_cast<float>(recorte.size.x);
    const float alto = static_cast<float>(recorte.size.y);
    const float escala = std::min(96.0f / ancho,
                                 (estacion ? 120.0f : 80.0f) / alto);
    figura.setOrigin(_puntosApoyo[indice][objeto.usado ? 1 : 0]);
    figura.setScale({escala, escala});
    figura.setPosition({objeto.posicion.x, objeto.posicion.y + 28.0f});
    destino.draw(figura);
}

void AspectoEncuentros::dibujarRespaldo(sf::RenderTarget& destino,
                                      const ObjetoEncuentro& objeto) const
{
    const bool estacion = objeto.tipo == TipoEncuentro::Estacion;
    const float alto = estacion ? 120.0f : 80.0f;
    const float izquierda = objeto.posicion.x - 48.0f;
    const float arriba = objeto.posicion.y + 28.0f - alto;
    sf::RectangleShape cuerpo({96.0f, alto});
    cuerpo.setPosition({izquierda, arriba});
    cuerpo.setFillColor(sf::Color(60, 69, 76));
    cuerpo.setOutlineThickness(2.0f);
    cuerpo.setOutlineColor(sf::Color(125, 135, 140));
    destino.draw(cuerpo);
    sf::RectangleShape panel({64.0f, estacion ? 42.0f : 36.0f});
    panel.setPosition({izquierda + 16.0f, arriba + 14.0f});
    panel.setFillColor(objeto.usado ? sf::Color(15, 20, 24) :
        (estacion ? sf::Color(30, 150, 100) : sf::Color(100, 112, 120)));
    destino.draw(panel);
    sf::RectangleShape banda({72.0f, 10.0f});
    banda.setPosition({izquierda + 12.0f, objeto.posicion.y + 10.0f});
    banda.setFillColor(sf::Color(210, 155, 40));
    destino.draw(banda);
    if (!objeto.usado)
    {
        sf::RectangleShape luz({12.0f, 6.0f});
        luz.setPosition({izquierda + 42.0f, objeto.posicion.y});
        luz.setFillColor(estacion ? sf::Color(85, 235, 165) :
                                   sf::Color(255, 200, 70));
        destino.draw(luz);
    }
}
