#include <SFML/Graphics.hpp>
#include <array>
#include <cstddef>
#include <vector>
#include <iostream>
#include <limits>
#include <cmath>
#include "Arma.h"
#include "Jugador.h"
#include "Enemigo.h"
#include <random>
#include "EncuentroAleatorio.h"
#define private public
#include "Juego.h"
#undef private

namespace
{
void irAEstacion(Juego& juego)
{
    for (const auto& entrada : juego._encuentros.obtenerZonas())
    {
        if (entrada.second[0].presente && !entrada.second[0].usado)
        {
            juego.jugador.setPosicion(entrada.second[0].posicion);
            return;
        }
    }
}
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
    juego._encuentros = EncuentroAleatorio(42);
    for (int i = 0; i < 5; ++i)
    {
        juego._encuentros.generarAlrededor({i * 6000.0f, 0.0f});
    }
    irAEstacion(juego);
    const Posicion estacion = juego.jugador.getPosicion();
    juego.jugador.setPosicion({estacion.x + 101.0f, estacion.y});
    juego.jugador.ganarChatarra(200);
    juego.intentarComprarMejora();
    correcto &= comprobar(juego.jugador.getChatarra() == 200 &&
                          !juego._estacionMejoras.menuAbierto(), "lejos no compra ni cobra");
    juego.jugador.setPosicion(estacion);
    juego.jugador.gastarChatarra(191);
    juego.intentarComprarMejora();
    correcto &= comprobar(juego.jugador.getChatarra() == 9 &&
                          juego._estacionMejoras.obtenerCosto() == 10 &&
                          !juego._estacionMejoras.menuAbierto(), "saldo insuficiente no cobra ni aumenta costo");
    juego.jugador.ganarChatarra(191);
    juego.intentarComprarMejora();
    juego.intentarComprarMejora();
    correcto &= comprobar(juego.jugador.getChatarra() == 190 &&
                          juego._estacionMejoras.menuAbierto(), "abrir cobra exactamente una vez");
    juego._proyectiles.emplace_back(Posicion{500.0f, 400.0f}, 100.0f, 0.0f, 8.0f, 4.0f, 10);
    const Posicion enemigoAntes = juego.enemigosPrueba[0].obtenerPosicion();
    juego.actualizar();
    juego.actualizarDisparoAutomatico();
    correcto &= comprobar(juego._proyectiles.size() == 1 &&
                          juego._proyectiles.front().obtenerPosicion().x == 500.0f &&
                          juego.enemigosPrueba[0].obtenerPosicion().x == enemigoAntes.x,
                          "menu pausa simulacion y no dispara");
    juego.elegirMejoraEstacion(3);
    correcto &= comprobar(juego._estacionMejoras.menuAbierto() &&
                          juego._estacionMejoras.obtenerCosto() == 10,
                          "eleccion invalida no consume compra");
    juego.dibujar();
    juego.jugador.recibirDano(20);
    juego.elegirMejoraEstacion(0);
    juego.elegirMejoraEstacion(0);
    correcto &= comprobar(juego.jugador.getVidaMaxima() == 120 && juego.jugador.getVida() == 100 &&
                          juego._estacionMejoras.obtenerCosto() == 16 &&
                          !juego._estacionMejoras.menuAbierto(), "una sola mejora vida y costo 16");
    irAEstacion(juego);
    juego.intentarComprarMejora();
    juego.elegirMejoraEstacion(1);
    correcto &= comprobar(juego.armaPrueba.obtenerDano() == 30 &&
                          juego._metralletaPrueba.obtenerDano() == 8 &&
                          juego._estacionMejoras.obtenerCosto() == 26 &&
                          juego.jugador.getChatarra() == 174, "potencia ambas armas y costo 26");
    irAEstacion(juego);
    juego.intentarComprarMejora();
    juego.elegirMejoraEstacion(2);
    correcto &= comprobar(std::abs(juego.armaPrueba.obtenerVelocidadAtaque() - 2.3f) < 0.001f &&
                          std::abs(juego._metralletaPrueba.obtenerVelocidadAtaque() - 9.2f) < 0.001f &&
                          juego._estacionMejoras.obtenerCosto() == 42 &&
                          juego.jugador.getChatarra() == 148, "cadencia ambas armas y costo 42");
    irAEstacion(juego);
    juego.intentarComprarMejora();
    correcto &= comprobar(juego.jugador.getChatarra() == 106, "pago de 42 antes de elegir");
    juego.reiniciarPrueba();
    correcto &= comprobar(juego.jugador.getChatarra() == 148 &&
                          juego._estacionMejoras.obtenerCosto() == 42 &&
                          juego.armaPrueba.obtenerDano() == 30 && juego.jugador.getVidaMaxima() == 120 &&
                          !juego._estacionMejoras.menuAbierto(), "reinicio devuelve pendiente y conserva compras confirmadas");
    juego.jugador.setPosicion(estacion);
    juego.jugador.recibirDano(999);
    juego.intentarComprarMejora();
    correcto &= comprobar(!juego._estacionMejoras.menuAbierto() && juego.jugador.getChatarra() == 148,
                          "jugador muerto no compra");
    const Posicion actual = juego._estacionMejoras.obtenerPosicion();
    correcto &= comprobar(juego._estacionMejoras.estaCerca({actual.x + 100.0f, actual.y}) &&
                          !juego._estacionMejoras.estaCerca({actual.x + 101.0f, actual.y}),
                          "radio de interaccion incluye borde");
    juego._estacionMejoras._costo = std::numeric_limits<int>::max();
    juego._estacionMejoras.abrirMenu();
    MejoraBasica opcion{};
    correcto &= comprobar(juego._estacionMejoras.confirmarEleccion(0, opcion) &&
                          juego._estacionMejoras.obtenerCosto() == std::numeric_limits<int>::max(),
                          "precio grande no desborda");
    juego.ventana.close();
    if (correcto)
    {
        std::cout << "Estacion: cobro, precio, pausa, mejoras y reembolso correctos\n";
    }
    return correcto ? 0 : 1;
}
