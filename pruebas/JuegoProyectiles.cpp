#include "Arma.h"
#include "Enemigo.h"
#include "Jugador.h"

#include <SFML/Graphics.hpp>
#include <array>
#include <cstddef>
#include <vector>
#include <iostream>

// Acceso limitado a esta prueba para comprobar el ciclo sin ampliar la API del juego.
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
    juego.actualizarDisparoAutomatico();
    correcto &= comprobar(juego._proyectiles.size() == 1, "primer disparo");
    juego.actualizarProyectiles(0.1f);
    const Posicion posicionPrimero = juego._proyectiles.front().obtenerPosicion();
    juego.armaPrueba.actualizar(0.25f);
    juego.actualizarDisparoAutomatico();
    correcto &= comprobar(juego._proyectiles.size() == 1, "cadencia bloquea disparo anticipado");
    juego.armaPrueba.actualizar(0.25f);
    juego.actualizarDisparoAutomatico();
    correcto &= comprobar(juego._proyectiles.size() == 2, "dos disparos activos simultaneos");
    if (juego._proyectiles.size() == 2)
    {
        correcto &= comprobar(
            juego._proyectiles.front().obtenerPosicion().x == posicionPrimero.x &&
            juego._proyectiles.front().obtenerPosicion().y == posicionPrimero.y &&
            juego._proyectiles.back().obtenerPosicion().x != posicionPrimero.x,
            "nuevo disparo conserva el anterior y nace en el jugador");
    }
    juego.dibujar();
    juego.reiniciarPrueba();
    correcto &= comprobar(juego._proyectiles.empty() && juego.armaPrueba.puedeDisparar(),
                          "reinicio limpia proyectiles y cadencia");

    for (Enemigo& enemigo : juego.enemigosPrueba)
    {
        enemigo = Enemigo(1, 100.0f, 100.0f, 56.0f, 56.0f,
                          0.0f, 25, 10, "Prueba", 10, 5);
    }
    juego._proyectiles.emplace_back(Posicion{110.0f, 110.0f}, 0.0f, 0.0f, 8.0f, 4.0f, 25);
    juego.actualizarProyectiles(0.01f);
    correcto &= comprobar(!juego.enemigosPrueba[0].estaVivo() &&
                          juego.enemigosPrueba[1].estaVivo() &&
                          juego.enemigosPrueba[2].estaVivo() && juego._proyectiles.empty(),
                          "un proyectil impacta solo al primer enemigo");
    correcto &= comprobar(juego.recursosSuelo.size() == 2 && juego.tiemposReaparicion[0] == 3.0f,
                          "una recompensa y temporizador por muerte");
    juego.actualizarProyectiles(0.01f);
    correcto &= comprobar(juego.recursosSuelo.size() == 2, "impacto eliminado no repite recompensa");

    juego._proyectiles.emplace_back(Posicion{110.0f, 110.0f}, 0.0f, 0.0f, 8.0f, 4.0f, 25);
    juego._proyectiles.emplace_back(Posicion{110.0f, 110.0f}, 0.0f, 0.0f, 8.0f, 4.0f, 25);
    juego.actualizarProyectiles(0.01f);
    correcto &= comprobar(!juego.enemigosPrueba[1].estaVivo() &&
                          !juego.enemigosPrueba[2].estaVivo() &&
                          juego.recursosSuelo.size() == 6 && juego._proyectiles.empty(),
                          "disparos sucesivos omiten muertos y no duplican recompensas");

    juego._proyectiles.emplace_back(Posicion{1910.0f, 20.0f}, 100.0f, 0.0f, 8.0f, 4.0f, 25);
    juego._proyectiles.emplace_back(Posicion{20.0f, 20.0f}, 100.0f, 0.0f, 8.0f, 4.0f, 25);
    juego.actualizarProyectiles(0.1f);
    correcto &= comprobar(juego._proyectiles.size() == 1 &&
                          juego._proyectiles.front().obtenerPosicion().x == 30.0f,
                          "limpieza fuera de pantalla conserva disparo activo");
    juego._proyectiles.front().desactivar();
    juego.actualizarProyectiles(0.1f);
    correcto &= comprobar(juego._proyectiles.empty(), "limpieza de disparos ya inactivos");
    juego.jugador.recibirDano(999);
    juego.armaPrueba.reiniciar();
    juego.actualizarDisparoAutomatico();
    correcto &= comprobar(juego._proyectiles.empty(), "jugador muerto no dispara");
    juego.reiniciarPrueba();
    juego.actualizarDisparoAutomatico();
    correcto &= comprobar(juego._proyectiles.size() == 1, "disparo inmediato despues de reiniciar");

    juego.seleccionarArmaPrueba(Juego::TipoArmaPrueba::Metralleta);
    correcto &= comprobar(juego.obtenerArmaSeleccionada().obtenerId() == 2 &&
                          juego.obtenerArmaSeleccionada().obtenerDano() == 6 &&
                          juego.obtenerArmaSeleccionada().obtenerVelocidadAtaque() == 8.0f &&
                          juego.obtenerArmaSeleccionada().obtenerAlcance() == 2210.0f,
                          "estadisticas de metralleta y seleccion");
    juego.actualizarDisparoAutomatico();
    correcto &= comprobar(juego._proyectiles.size() == 2 &&
                          juego._proyectiles.front().obtenerDano() == 25 &&
                          juego._proyectiles.back().obtenerDano() == 6 &&
                          juego._proyectiles.back().obtenerFormaColision().ancho == 14.0f &&
                          juego._proyectiles.back().obtenerFormaColision().alto == 6.0f,
                          "ambas armas conservan sus disparos y dimensiones");
    juego.seleccionarArmaPrueba(Juego::TipoArmaPrueba::Pistola);
    juego.actualizarDisparoAutomatico();
    correcto &= comprobar(juego._proyectiles.size() == 2, "alternar no reinicia cooldown de pistola");
    juego.dibujar();
    correcto &= comprobar(juego.figuraProyectilPrueba.getFillColor() == sf::Color(255, 140, 50) &&
                          juego.figuraProyectilPrueba.getSize() == sf::Vector2f(14.0f, 6.0f),
                          "disparo de metralleta sigue naranja y pequeno al elegir pistola");
    juego.seleccionarArmaPrueba(Juego::TipoArmaPrueba::Metralleta);
    juego.actualizarArmas(0.0625f);
    juego.actualizarDisparoAutomatico();
    correcto &= comprobar(juego._proyectiles.size() == 2, "metralleta no dispara antes de 0.125 segundos");
    juego.actualizarArmas(0.0625f);
    juego.actualizarDisparoAutomatico();
    correcto &= comprobar(juego._proyectiles.size() == 3, "metralleta dispara cada 0.125 segundos");
    juego.actualizarArmas(0.375f);
    juego.seleccionarArmaPrueba(Juego::TipoArmaPrueba::Pistola);
    juego.actualizarDisparoAutomatico();
    correcto &= comprobar(juego._proyectiles.size() == 4 &&
                          juego._proyectiles.back().obtenerDano() == 25 &&
                          juego._proyectiles.back().obtenerFormaColision().ancho == 28.0f,
                          "pistola recupera cooldown estando inactiva y conserva su formato");
    juego.dibujar();
    correcto &= comprobar(juego.figuraProyectilPrueba.getFillColor() == sf::Color(255, 225, 70) &&
                          juego.figuraProyectilPrueba.getSize() == sf::Vector2f(28.0f, 14.0f),
                          "disparo de pistola conserva apariencia despues de metralleta");
    juego.seleccionarArmaPrueba(Juego::TipoArmaPrueba::Metralleta);
    juego.reiniciarPrueba();
    correcto &= comprobar(juego.obtenerArmaSeleccionada().obtenerId() == 1 &&
                          juego.armaPrueba.puedeDisparar() && juego._metralletaPrueba.puedeDisparar() &&
                          juego._proyectiles.empty(), "reinicio restaura ambas armas y selecciona pistola");

    for (Enemigo& enemigo : juego.enemigosPrueba)
    {
        enemigo.recibirDanio(999);
    }
    juego.enemigosPrueba[0] = Enemigo(1, 100.0f, 100.0f, 56.0f, 56.0f,
                                    0.0f, 7, 10, "Prueba metralleta", 10, 5);
    juego.jugador.setPosicion({50.0f, 128.0f});
    juego.seleccionarArmaPrueba(Juego::TipoArmaPrueba::Metralleta);
    juego.actualizarDisparoAutomatico();
    const Posicion origenMetralleta = juego._proyectiles.front().obtenerPosicion();
    juego.actualizarProyectiles(0.075f);
    correcto &= comprobar(juego.enemigosPrueba[0].estaVivo() && juego._proyectiles.empty(),
                          "un disparo de seis no mata enemigo con siete de vida");
    juego.actualizarArmas(0.125f);
    juego.actualizarDisparoAutomatico();
    correcto &= comprobar(juego._proyectiles.front().obtenerPosicion().x == origenMetralleta.x,
                          "siguiente disparo nace en el mismo origen");
    juego.actualizarProyectiles(0.075f);
    correcto &= comprobar(!juego.enemigosPrueba[0].estaVivo() && juego.recursosSuelo.size() == 2,
                          "segundo disparo mata y entrega una recompensa");
    juego.ventana.close();
    if (correcto)
    {
        std::cout << "Juego: multiples proyectiles, pruebas correctas\n";
    }
    return correcto ? 0 : 1;
}
