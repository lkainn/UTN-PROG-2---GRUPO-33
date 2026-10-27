#include "AspectoEncuentros.h"
#include "EntornoDatosPrueba.h"

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <limits>

namespace
{
bool comprobar(bool condicion, const char* caso)
{
    if (!condicion)
    {
        std::cerr << "Fallo: " << caso << '\n';
    }
    return condicion;
}

sf::Image renderizar(const AspectoEncuentros& aspecto, TipoEncuentro tipo,
                     bool usado)
{
    sf::RenderTexture superficie({250u, 250u});
    superficie.clear(sf::Color::Transparent);
    aspecto.dibujar(superficie, {tipo, {125.0f, 180.0f}, usado});
    superficie.display();
    return superficie.getTexture().copyToImage();
}

sf::IntRect limitesVisibles(const sf::Image& imagen)
{
    int izquierda = 250;
    int arriba = 250;
    int derecha = -1;
    int abajo = -1;
    for (unsigned int y = 0; y < 250; ++y)
    {
        for (unsigned int x = 0; x < 250; ++x)
        {
            if (imagen.getPixel({x, y}).a >= 32)
            {
                izquierda = std::min(izquierda, static_cast<int>(x));
                arriba = std::min(arriba, static_cast<int>(y));
                derecha = std::max(derecha, static_cast<int>(x));
                abajo = std::max(abajo, static_cast<int>(y));
            }
        }
    }
    return sf::IntRect({izquierda, arriba},
        {derecha - izquierda + 1, abajo - arriba + 1});
}
}

int main()
{
    bool correcto = true;
    AspectoEncuentros aspecto;
    AspectoEncuentros respaldo("recursos/no_existe");
    const std::array<TipoEncuentro, 2> TIPOS{{
        TipoEncuentro::Cofre, TipoEncuentro::Estacion
    }};
    for (TipoEncuentro tipo : TIPOS)
    {
        correcto &= comprobar(aspecto.texturaDisponible(tipo), "carga de hoja");
        correcto &= comprobar(!respaldo.texturaDisponible(tipo), "respaldo sin PNG");
        const sf::Image disponible = renderizar(aspecto, tipo, false);
        const sf::Image usado = renderizar(aspecto, tipo, true);
        const sf::IntRect area = limitesVisibles(disponible);
        const sf::IntRect areaUsada = limitesVisibles(usado);
        correcto &= comprobar(area.size.x >= 55 && area.size.x <= 97 &&
            area.size.y >= 45 && area.size.y <=
                (tipo == TipoEncuentro::Estacion ? 121 : 81),
            "dimensiones a escala de juego");
        correcto &= comprobar(disponible.getPixel({0u, 0u}).a == 0 &&
            disponible.getPixel({125u, 180u}).a > 0,
            "transparencia fuera y cuerpo visible");
        correcto &= comprobar(std::abs(area.position.x - areaUsada.position.x) <= 4 &&
            std::abs(area.size.x - areaUsada.size.x) <= 4 &&
            std::abs(area.position.y + area.size.y -
                     areaUsada.position.y - areaUsada.size.y) <= 4,
            "estado usado mantiene base y escala");
        int diferencias = 0;
        for (unsigned int y = 0; y < 250; ++y)
        {
            for (unsigned int x = 0; x < 250; ++x)
            {
                if (disponible.getPixel({x, y}) != usado.getPixel({x, y}))
                {
                    ++diferencias;
                }
            }
        }
        correcto &= comprobar(diferencias > 50, "estados visualmente distintos");
        const sf::IntRect areaRespaldo = limitesVisibles(renderizar(respaldo, tipo, false));
        correcto &= comprobar(areaRespaldo.size.x > 80 && areaRespaldo.size.y > 60,
            "respaldo industrial visible");
    }
    sf::RenderTexture vacia({250u, 250u});
    vacia.clear(sf::Color::Transparent);
    aspecto.dibujar(vacia, {TipoEncuentro::Cofre, {125.0f, 180.0f}, false, false});
    aspecto.dibujar(vacia, {TipoEncuentro::Estacion,
        {std::numeric_limits<float>::infinity(), 180.0f}, false});
    vacia.display();
    correcto &= comprobar(limitesVisibles(vacia.getTexture().copyToImage()).size.x <= 0,
        "ausentes y posiciones invalidas no dibujan");

    const bool desdeBin = std::filesystem::exists("../../recursos/Tuffy.ttf");
    const std::string raiz = desdeBin ? "../../" : "";
    sf::Texture suelo;
    sf::Font fuente;
    correcto &= comprobar(suelo.loadFromFile(raiz + "recursos/entorno/suelo_chatarra_v1.png") &&
                          fuente.openFromFile(raiz + "recursos/Tuffy.ttf"), "recursos de vista");
    sf::RenderTexture vista({900u, 380u});
    sf::RectangleShape fondo({1920.0f, 1080.0f});
    fondo.setTexture(&suelo, true);
    vista.draw(fondo);
    const std::array<ObjetoEncuentro, 4> OBJETOS{{
        {TipoEncuentro::Cofre, {130.0f, 200.0f}, false},
        {TipoEncuentro::Cofre, {340.0f, 200.0f}, true},
        {TipoEncuentro::Estacion, {550.0f, 200.0f}, false},
        {TipoEncuentro::Estacion, {760.0f, 200.0f}, true}
    }};
    const std::array<const char*, 4> ETIQUETAS{{
        "CONTENEDOR", "VACIO", "MEJORAS", "DESACTIVADA"
    }};
    for (std::size_t indice = 0; indice < OBJETOS.size(); ++indice)
    {
        aspecto.dibujar(vista, OBJETOS[indice], indice == 0 || indice == 2, true);
        sf::Text etiqueta(fuente, ETIQUETAS[indice], 20);
        const sf::FloatRect area = etiqueta.getLocalBounds();
        etiqueta.setOrigin({area.position.x + area.size.x / 2.0f, area.position.y});
        etiqueta.setPosition({OBJETOS[indice].posicion.x, 250.0f});
        etiqueta.setOutlineColor(sf::Color::Black);
        etiqueta.setOutlineThickness(1.0f);
        vista.draw(etiqueta);
    }
    vista.display();
    std::filesystem::create_directories(obtenerCarpetaSalidasPrueba());
    correcto &= comprobar(vista.getTexture().copyToImage().saveToFile(
        obtenerCarpetaSalidasPrueba() + "/encuentros_vista.png"), "vista sobre suelo");
    std::cout << (correcto ? "Aspecto encuentros OK\n" : "Aspecto encuentros FALLO\n");
    return correcto ? 0 : 1;
}
