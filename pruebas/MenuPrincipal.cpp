#include <SFML/Graphics.hpp>
#include <array>
#include <cstddef>
#include <vector>
#include <iostream>
#include <limits>
#include <cmath>
#include "GestorOleadas.h"
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
    correcto &= comprobar(juego.fuenteDanioDisponible, "fuente del menu disponible");
    const Posicion posicion = juego.jugador.getPosicion();
    const auto enemigos = juego._gestorOleadas.obtenerEnemigos().size();
    juego.actualizar();
    correcto &= comprobar(juego.jugador.getPosicion().x == posicion.x &&
                          juego.jugador.getPosicion().y == posicion.y &&
                          juego._gestorOleadas.obtenerEnemigos().size() == enemigos &&
                          juego._proyectiles.empty(), "menu no avanza combate");
    juego._escenario.actualizar({-2000.0f, 4000.0f});
    juego.dibujar();
    juego.dibujar();
    correcto &= comprobar(juego.ventana.getView().getCenter() ==
                          juego.ventana.getDefaultView().getCenter(),
                          "menu usa coordenadas de pantalla");
    sf::Texture captura(juego.ventana.getSize());
    captura.update(juego.ventana);
    correcto &= comprobar(captura.copyToImage().saveToFile(
                          ".codex_tmp/menu_principal.png"), "captura del menu");

    juego.procesarEntradaMenu(sf::Keyboard::Scancode::Up);
    correcto &= comprobar(juego._opcionMenu == 2, "navegacion circular hacia arriba");
    juego.procesarEntradaMenu(sf::Keyboard::Scancode::Down);
    juego.procesarEntradaMenu(sf::Keyboard::Scancode::Down);
    juego.procesarEntradaMenu(sf::Keyboard::Scancode::Enter);
    juego.procesarEntradaMenu(sf::Keyboard::Scancode::Down);
    juego.procesarEntradaMenu(sf::Keyboard::Scancode::Escape);
    correcto &= comprobar(juego.ventana.isOpen() && juego._opcionMenu == 1 &&
                          juego._estado == Juego::EstadoJuego::Menu,
                          "controles vuelve sin cerrar ni cambiar seleccion");
    juego.procesarEntradaMenu(sf::Keyboard::Scancode::Enter);
    juego.dibujar();
    juego.dibujar();
    captura.update(juego.ventana);
    correcto &= comprobar(captura.copyToImage().saveToFile(
                          ".codex_tmp/menu_controles.png"), "captura de controles");
    juego.procesarEntradaMenu(sf::Keyboard::Scancode::Enter);
    juego.procesarEntradaMenu(sf::Keyboard::Scancode::Up);
    sf::sleep(sf::milliseconds(100));
    juego.procesarEntradaMenu(sf::Keyboard::Scancode::Enter);
    correcto &= comprobar(juego._estado == Juego::EstadoJuego::Jugando &&
                          juego.relojFotogramas.getElapsedTime().asSeconds() < 0.05f,
                          "jugar descarta espera del menu");
    juego.actualizar();
    juego.dibujar();
    juego.dibujar();

    juego._proyectiles.clear();
    juego._proyectiles.emplace_back(Posicion{500.0f, 400.0f},
        100.0f, 0.0f, 8.0f, 4.0f, 10);
    juego.armaPrueba.reiniciar();
    Proyectil disparo({500.0f, 400.0f}, 100.0f, 0.0f, 8.0f, 4.0f, 10);
    disparo.desactivar();
    juego.armaPrueba.intentarDisparar(disparo, {500.0f, 400.0f},
        {600.0f, 400.0f}, 100.0f, 8.0f, 4.0f);
    juego.tiempoRestanteDanoContacto = 0.4f;
    const Posicion posicionPausa = juego.jugador.getPosicion();
    const int vidaPausa = juego.jugador.getVida();
    const int chatarraPausa = juego.jugador.getChatarra();
    const int armaPausa = juego._inventarioArmas.obtenerIdSlot(0);
    const float tiempoPausa = juego._gestorOleadas.tiempoDePartida();
    juego.manejarEscape();
    correcto &= comprobar(juego._estado == Juego::EstadoJuego::Pausa &&
                          juego.ventana.isOpen() && juego._opcionMenu == 0,
                          "Escape abre pausa sin cerrar");
    sf::sleep(sf::milliseconds(100));
    for (int fotograma = 0; fotograma < 3; ++fotograma)
    {
        juego.actualizar();
        juego.dibujar();
    }
    correcto &= comprobar(juego.jugador.getPosicion().x == posicionPausa.x &&
                          juego.jugador.getPosicion().y == posicionPausa.y &&
                          juego.jugador.getVida() == vidaPausa &&
                          juego.jugador.getChatarra() == chatarraPausa &&
                          juego._inventarioArmas.obtenerIdSlot(0) == armaPausa &&
                          juego._gestorOleadas.tiempoDePartida() == tiempoPausa &&
                          juego._proyectiles.size() == 1 &&
                          juego._proyectiles.front().obtenerPosicion().x == 500.0f &&
                          !juego.armaPrueba.puedeDisparar() &&
                          juego.tiempoRestanteDanoContacto == 0.4f,
                          "pausa conserva partida, proyectil y recargas");
    captura.update(juego.ventana);
    correcto &= comprobar(captura.copyToImage().saveToFile(
                          ".codex_tmp/menu_pausa.png"), "captura de pausa");
    juego.procesarEntradaMenu(sf::Keyboard::Scancode::Down);
    juego.procesarEntradaMenu(sf::Keyboard::Scancode::Escape);
    correcto &= comprobar(juego._estado == Juego::EstadoJuego::Jugando &&
                          juego.relojFotogramas.getElapsedTime().asSeconds() < 0.05f &&
                          juego._proyectiles.size() == 1,
                          "Escape continua aun con Salir seleccionado");
    juego.manejarEscape();
    juego.procesarEntradaMenu(sf::Keyboard::Scancode::Enter);
    correcto &= comprobar(juego._estado == Juego::EstadoJuego::Jugando &&
                          juego._proyectiles.size() == 1,
                          "Continuar no reinicia partida");

    for (const auto& entrada : juego._encuentros.obtenerZonas())
    {
        if (entrada.second[0].presente && !entrada.second[0].usado)
        {
            juego.jugador.setPosicion(entrada.second[0].posicion);
            break;
        }
    }
    juego.jugador.ganarChatarra(500);
    const int saldoAntes = juego.jugador.getChatarra();
    juego.intentarComprarMejora();
    correcto &= comprobar(juego._estacionMejoras.menuAbierto(),
                          "estacion abierta para probar Escape");
    juego.manejarEscape();
    correcto &= comprobar(!juego._estacionMejoras.menuAbierto() &&
                          juego._estado == Juego::EstadoJuego::Jugando &&
                          juego.jugador.getChatarra() == saldoAntes,
                          "Escape cancela compra y reembolsa sin abrir pausa");

    Juego salida;
    salida.ventana.setVisible(false);
    salida.procesarEntradaMenu(sf::Keyboard::Scancode::Enter);
    salida.manejarEscape();
    salida.procesarEntradaMenu(sf::Keyboard::Scancode::Up);
    salida.procesarEntradaMenu(sf::Keyboard::Scancode::Enter);
    correcto &= comprobar(!salida.ventana.isOpen(), "salir cierra ventana");
    std::cout << (correcto ? "Menu principal OK\n" : "Menu principal FALLO\n");
    return correcto ? 0 : 1;
}
