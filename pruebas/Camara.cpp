#include <SFML/Graphics.hpp>
#include <array>
#include <cstddef>
#include <vector>
#include <iostream>
#include <limits>
#include "Arma.h"
#include "Enemigo.h"
#include "Jugador.h"

#include <random>
#include "EncuentroAleatorio.h"
#define private public
#include "Juego.h"
#undef private

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
    Juego juego;
    juego.ventana.setVisible(false);
    correcto &= comprobar(juego._escenario.fondoDisponible(), "suelo disponible");
    FormaColision area = juego._escenario.obtenerAreaVisible();
    correcto &= comprobar(area.x == 0.0f && area.y == 0.0f &&
                          area.ancho == 1920.0f && area.alto == 1080.0f,
                          "vista inicial compatible con el escenario anterior");
    juego.jugador.setPosicion({-300.0f, -200.0f});
    juego.jugador.actualizar(false);
    correcto &= comprobar(juego.jugador.getPosicion().x == -300.0f &&
                          juego.jugador.getFormaColision().y == -224.0f,
                          "jugador libre sincroniza forma en coordenadas negativas");

    sf::RenderTexture superficie({1920u, 1080u});
    const std::array<Posicion, 4> destinos{{
        {-300.0f, -200.0f}, {7000.0f, 4000.0f},
        {-1920.0f, -1080.0f}, {960.0f, 540.0f}
    }};
    for (const Posicion destino : destinos)
    {
        juego._escenario.actualizar(destino);
        const sf::Vector2i centro = juego.ventana.mapCoordsToPixel(
            {destino.x, destino.y}, juego._escenario.obtenerVista());
        correcto &= comprobar(centro == sf::Vector2i(960, 540), "camara sigue al centro");
        superficie.setView(juego._escenario.obtenerVista());
        superficie.clear(sf::Color::Magenta);
        juego._escenario.dibujar(superficie);
        superficie.display();
        const sf::Image imagen = superficie.getTexture().copyToImage();
        for (unsigned int y : {0u, 270u, 539u, 810u, 1079u})
        {
            for (unsigned int x : {0u, 480u, 959u, 1440u, 1919u})
            {
                correcto &= comprobar(imagen.getPixel({x, y}) != sf::Color::Magenta,
                                      "suelo cubre vista sin franjas vacias");
            }
        }
    }
    const sf::Vector2f centroAnterior = juego._escenario.obtenerVista().getCenter();
    juego._escenario.actualizar({std::numeric_limits<float>::infinity(), 0.0f});
    correcto &= comprobar(juego._escenario.obtenerVista().getCenter() == centroAnterior,
                          "coordenadas no finitas no contaminan camara");

    juego._escenario.actualizar({960.0f, 540.0f});
    superficie.setView(juego._escenario.obtenerVista());
    juego._escenario.dibujar(superficie);
    superficie.display();
    const sf::Color pixelMundo = superficie.getTexture().copyToImage().getPixel({900u, 500u});
    juego._escenario.actualizar({1080.0f, 620.0f});
    superficie.setView(juego._escenario.obtenerVista());
    juego._escenario.dibujar(superficie);
    superficie.display();
    correcto &= comprobar(superficie.getTexture().copyToImage().getPixel({780u, 420u}) == pixelMundo,
                          "suelo permanece anclado al mundo al mover la camara");

    juego.jugador.setPosicion({7000.0f, 4000.0f});
    juego.jugador.actualizar(false);
    juego._escenario.actualizar(juego.jugador.getPosicion());
    juego._proyectiles.emplace_back(Posicion{6100.0f, 3500.0f}, 0.0f, 0.0f, 8.0f, 4.0f, 25);
    juego._proyectiles.emplace_back(Posicion{20.0f, 20.0f}, 0.0f, 0.0f, 8.0f, 4.0f, 25);
    juego.actualizarProyectiles(0.01f);
    correcto &= comprobar(juego._proyectiles.size() == 1 &&
                          juego._proyectiles.front().obtenerPosicion().x == 6100.0f,
                          "limpieza de proyectiles usa area visible desplazada");
    for (std::size_t i = 0; i < juego.enemigosPrueba.size(); ++i)
    {
        juego.enemigosPrueba[i].recibirDanio(999);
        juego.tiemposReaparicion[i] = 0.0f;
    }
    juego.actualizarEnemigos(0.01f);
    correcto &= comprobar(juego.enemigosPrueba[0].obtenerPosicion().x == 7480.0f &&
                          juego.enemigosPrueba[0].obtenerPosicion().y == 3660.0f,
                          "enemigos reaparecen respecto al jugador desplazado");
    juego.dibujar();
    correcto &= comprobar(juego.ventana.getView().getCenter() ==
                          juego.ventana.getDefaultView().getCenter(),
                          "interfaz restaura vista fija");
    juego.reiniciarPrueba();
    area = juego._escenario.obtenerAreaVisible();
    correcto &= comprobar(area.x == 0.0f && area.y == 0.0f && juego._proyectiles.empty(),
                          "reinicio restaura camara y entidades");
    juego.jugador.setPosicion({-300.0f, -200.0f});
    juego.jugador.actualizar();
    correcto &= comprobar(juego.jugador.getFormaColision().x == 0.0f &&
                          juego.jugador.getFormaColision().y == 0.0f,
                          "actualizar por defecto conserva limites del modulo Jugador");
    juego.ventana.close();
    if (correcto)
    {
        std::cout << "Camara: seguimiento, suelo, coordenadas y reinicio correctos\n";
    }
    return correcto ? 0 : 1;
}
