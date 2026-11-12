#include <SFML/Graphics.hpp>
#include "EntornoDatosPrueba.h"
#include "CarpetaDatos.h"
#include "Arma.h"
#include "Jugador.h"
#include "GestorOleadas.h"
#include "EncuentroAleatorio.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <iostream>
#include <random>
#include <vector>

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

bool mismasImagenes(const sf::Image& primera, const sf::Image& segunda)
{
    const sf::Vector2u tamanio = primera.getSize();
    return tamanio == segunda.getSize() &&
        std::memcmp(primera.getPixelsPtr(), segunda.getPixelsPtr(),
                    static_cast<std::size_t>(tamanio.x) * tamanio.y * 4) == 0;
}

sf::Image dibujarRadar(Juego& juego)
{
    // Completar los dos buffers de la ventana antes de comparar capturas.
    for (int cuadro = 0; cuadro < 2; ++cuadro)
    {
        juego.ventana.setView(juego.ventana.getDefaultView());
        juego.ventana.clear(sf::Color::Black);
        juego.dibujarMinimapa();
        juego.ventana.display();
    }
    sf::Texture captura(juego.ventana.getSize());
    captura.update(juego.ventana);
    return captura.copyToImage();
}

bool probarIconosYBordes()
{
    Minimapa minimapa;
    bool correcto = comprobar(
        std::all_of(minimapa._iconoCargado.begin(), minimapa._iconoCargado.end(),
                    [](bool cargado) { return cargado; }) &&
        std::all_of(minimapa._objetoCargado.begin(), minimapa._objetoCargado.end(),
                    [](bool cargado) { return cargado; }),
        "cargar los seis iconos desde el directorio de ejecucion");

    const Posicion jugador{100.0f, -200.0f};
    std::vector<Enemigo> enemigos{
        Enemigo(1, jugador.x + 1776.0f, jugador.y - 24.0f, 48, 48, 0, 100, 1, "Basico", 1, 1),
        Enemigo(4, jugador.x + 6000.0f, jugador.y + 6000.0f, 120, 120, 0, 3000, 1, "Jefe", 1, 1)
    };
    const std::vector<Posicion> cofres{{jugador.x - 1800.0f, jugador.y}};
    const std::vector<Posicion> estaciones{{jugador.x, jugador.y - 1800.0f}};
    const sf::Color exterior(14, 25, 39);
    sf::RenderTexture superficie({640u, 640u});
    const auto dibujar = [&]
    {
        superficie.clear(exterior);
        minimapa.dibujar(superficie, jugador, enemigos, cofres, estaciones);
        superficie.display();
        return superficie.getTexture().copyToImage();
    };
    const sf::Image imagen = dibujar();
    bool dentroDelRadar = true;
    // El panel ocupa de 342 a 622, con un pixel adicional de contorno.
    for (unsigned int y = 0; y < 640; ++y)
    {
        for (unsigned int x = 0; x < 640; ++x)
        {
            if ((x < 341 || x > 623 || y < 341 || y > 623) && imagen.getPixel({x, y}) != exterior)
            {
                dentroDelRadar = false;
            }
        }
    }
    correcto &= comprobar(dentroDelRadar, "jefe lejano, enemigos y objetos quedan dentro del panel");
    enemigos.emplace_back(2, jugador.x - 6000.0f, jugador.y, 40, 40, 0, 100, 1, "Lejano", 1, 1);
    correcto &= comprobar(mismasImagenes(imagen, dibujar()), "enemigo comun fuera de alcance no aparece");
    enemigos.back() = Enemigo(2, jugador.x - 120.0f, jugador.y, 40, 40, 0, 0, 1, "Muerto", 1, 1);
    correcto &= comprobar(mismasImagenes(imagen, dibujar()), "enemigo muerto no aparece");
    correcto &= comprobar(imagen.saveToFile(obtenerCarpetaSalidasPrueba() + "/minimapa_bordes.png"),
                          "captura del minimapa con marcas en los bordes");
    return correcto;
}

bool probarIntegracion()
{
    Juego juego;
    juego.ventana.setVisible(false);
    juego.iniciarNuevaPartida();
    juego._encuentros = EncuentroAleatorio(123u);
    juego._encuentros.generarAlrededor({0.0f, 0.0f});
    juego._encuentros.generarAlrededor({6000.0f, 6000.0f});
    ObjetoEncuentro* cofre = nullptr;
    for (const auto& zona : juego._encuentros.obtenerZonas())
    {
        for (const ObjetoEncuentro& objeto : zona.second)
        {
            if (objeto.presente && objeto.tipo == TipoEncuentro::Cofre)
            {
                cofre = juego._encuentros.buscarCercano(objeto.posicion, TipoEncuentro::Cofre);
                break;
            }
        }
        if (cofre != nullptr)
        {
            break;
        }
    }
    if (!comprobar(cofre != nullptr, "cofre disponible para comprobar el radar"))
    {
        return false;
    }
    juego.jugador.setPosicion({cofre->posicion.x + 250.0f, cofre->posicion.y});
    juego._escenario.actualizar(juego.jugador.getPosicion());
    juego._gestorOleadas.actualizar(juego.jugador.getPosicion(), 20.0f);
    const sf::Image antes = dibujarRadar(juego);
    cofre->usado = true;
    const sf::Image despues = dibujarRadar(juego);
    bool correcto = comprobar(!mismasImagenes(antes, despues), "el cofre usado desaparece del radar");
    if (!comprobar(juego.guardarPartida(), "guardar una partida con minimapa"))
    {
        return false;
    }
    juego._estado = Juego::EstadoJuego::Menu;
    juego._continuacionDisponible = true;
    juego.procesarEntradaMenu(sf::Keyboard::Scancode::Enter);
    correcto &= comprobar(juego._estado == Juego::EstadoJuego::Jugando &&
        mismasImagenes(despues, dibujarRadar(juego)),
        "Continuar reconstruye el mismo radar con enemigos y encuentros restaurados");
    juego.dibujar();
    juego.dibujar();
    sf::Texture captura(juego.ventana.getSize());
    captura.update(juego.ventana);
    correcto &= comprobar(captura.copyToImage().saveToFile(
        obtenerCarpetaSalidasPrueba() + "/minimapa_partida.png"), "captura del minimapa en la partida");
    return correcto;
}
}

int main(int argc, char* argv[])
{
    if (argc == 2 && std::string(argv[1]) == "--iconos")
    {
        const bool correcto = probarIconosYBordes();
        std::cout << (correcto ? "Minimapa desde bin/Debug OK\n" : "Minimapa FALLO\n");
        return correcto ? 0 : 1;
    }
    if (!prepararDatosPrueba())
    {
        return 1;
    }
    const bool iconosCorrectos = probarIconosYBordes();
    const bool integracionCorrecta = probarIntegracion();
    const bool correcto = iconosCorrectos && integracionCorrecta;
    std::cout << (correcto ? "Minimapa integrado OK\n" : "Minimapa FALLO\n");
    return correcto ? 0 : 1;
}
