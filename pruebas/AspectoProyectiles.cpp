#include "AspectoProyectiles.h"
#include <cmath>
#include <iostream>

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
}

int main()
{
    bool correcto = true;
    AspectoProyectiles aspecto;
    AspectoProyectiles sinArchivos("recursos/no_existe");
    correcto &= comprobar(aspecto.texturaDisponible(false) && aspecto.texturaDisponible(true),
        "carga ambas texturas");
    correcto &= comprobar(!sinArchivos.texturaDisponible(false) && !sinArchivos.texturaDisponible(true),
        "faltantes utilizan respaldo");
    const char* nombres[] = {"pistola_v1.png", "metralleta_v1.png"};
    for (const char* nombre : nombres)
    {
        sf::Image imagen;
        bool cargada = imagen.loadFromFile(std::string("recursos/proyectiles/") + nombre);
        if (!cargada)
        {
            cargada = imagen.loadFromFile(std::string("../../recursos/proyectiles/") + nombre);
        }
        correcto &= comprobar(cargada, "PNG legible");
        const sf::Vector2u tamanio = imagen.getSize();
        unsigned int transparentes = 0;
        unsigned int visibles = 0;
        for (unsigned int y = 0; y < tamanio.y; ++y)
        {
            for (unsigned int x = 0; x < tamanio.x; ++x)
            {
                if (imagen.getPixel({x, y}).a == 0)
                {
                    ++transparentes;
                }
                else
                {
                    ++visibles;
                }
            }
        }
        correcto &= comprobar(transparentes > 0 && visibles > 0, "transparencia real y bala visible");
        std::cout << nombre << ": " << tamanio.x << 'x' << tamanio.y << ", alfa verificado\n";
    }
    sf::RenderTexture destino({128, 128});
    Proyectil horizontal({50.0f, 50.0f}, 100.0f, 0.0f, 28.0f, 14.0f, 25);
    Proyectil vertical({50.0f, 50.0f}, 0.0f, 100.0f, 28.0f, 14.0f, 25);
    correcto &= comprobar(std::abs(horizontal.obtenerAngulo()) < 0.001f &&
        std::abs(vertical.obtenerAngulo() - 1.5707963f) < 0.001f, "angulo sigue velocidad");
    destino.clear(sf::Color::Transparent);
    aspecto.dibujar(destino, vertical);
    destino.display();
    const sf::Image captura = destino.getTexture().copyToImage();
    int anchoVisible = 0;
    int altoVisible = 0;
    for (unsigned int x = 0; x < 128; ++x)
    {
        anchoVisible += captura.getPixel({x, 57}).a > 0 ? 1 : 0;
    }
    for (unsigned int y = 0; y < 128; ++y)
    {
        altoVisible += captura.getPixel({64, y}).a > 0 ? 1 : 0;
    }
    correcto &= comprobar(altoVisible > anchoVisible && anchoVisible > 0,
        "textura vertical gira y conserva centro");
    correcto &= comprobar(vertical.obtenerFormaColision().ancho == 28.0f &&
        vertical.obtenerDano() == 25 && vertical.obtenerPosicion().x == 50.0f,
        "dibujo no modifica datos de combate");
    destino.clear(sf::Color::Transparent);
    sinArchivos.dibujar(destino, horizontal);
    destino.display();
    correcto &= comprobar(destino.getTexture().copyToImage().getPixel({64, 57}) == sf::Color(255, 225, 70),
        "respaldo amarillo visible");
    horizontal.desactivar();
    destino.clear(sf::Color::Transparent);
    aspecto.dibujar(destino, horizontal);
    destino.display();
    correcto &= comprobar(destino.getTexture().copyToImage().getPixel({64, 57}).a == 0,
        "inactivo no se dibuja");

    sf::RenderTexture muestra({640, 240});
    muestra.clear(sf::Color(48, 41, 33));
    sf::Font fuente;
    const bool fuenteDisponible = fuente.openFromFile("recursos/Tuffy.ttf");
    for (int fila = 0; fila < 2; ++fila)
    {
        if (fuenteDisponible)
        {
            sf::Text texto(fuente, fila == 0 ? "PISTOLA 28x14" : "METRALLETA 14x6", 18);
            texto.setPosition({12.0f, 12.0f + fila * 110.0f});
            muestra.draw(texto);
        }
        const float ancho = fila == 0 ? 28.0f : 14.0f;
        const float alto = fila == 0 ? 14.0f : 6.0f;
        for (int i = 0; i < 4; ++i)
        {
            const float angulo = i * 0.78539816f;
            Proyectil bala({55.0f + i * 140.0f, 55.0f + fila * 110.0f},
                std::cos(angulo) * 100.0f, std::sin(angulo) * 100.0f, ancho, alto, 10);
            aspecto.dibujar(muestra, bala);
        }
    }
    muestra.display();
    if (fuenteDisponible)
    {
        correcto &= comprobar(muestra.getTexture().copyToImage().saveToFile(".codex_tmp/proyectiles_vista.png"),
            "vista previa guardada");
    }
    std::cout << (correcto ? "Texturas: carga, alfa, rotacion y respaldo correctos\n" : "Hay fallos\n");
    return correcto ? 0 : 1;
}
