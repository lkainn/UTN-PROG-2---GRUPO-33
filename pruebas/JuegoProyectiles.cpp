#include "Arma.h"
#include "Enemigo.h"
#include "Jugador.h"
#include "GestorOleadas.h"

#include <SFML/Graphics.hpp>
#include <array>
#include <cstddef>
#include <vector>
#include <iostream>
#include <cmath>
#include <algorithm>

// Acceso limitado a esta prueba para comprobar el ciclo sin ampliar la API del juego.
#include <random>
#include "EncuentroAleatorio.h"
#define private public
#include "Juego.h"
#undef private

namespace
{
void poblarEnemigos(Juego& juego)
{
    juego._gestorOleadas.obtenerEnemigos().assign(
        3, crearEnemigo(BASICO, 1440.0f, 512.0f));
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
    poblarEnemigos(juego);
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
            "nuevo disparo conserva el anterior y nace en la boca del arma");
    }
    juego.dibujar();
    juego.reiniciarPrueba();
    poblarEnemigos(juego);
    correcto &= comprobar(juego._proyectiles.empty() && juego.armaPrueba.puedeDisparar(),
                          "reinicio limpia proyectiles y cadencia");

    for (Enemigo& enemigo : juego._gestorOleadas.obtenerEnemigos())
    {
        enemigo = Enemigo(1, 100.0f, 100.0f, 56.0f, 56.0f,
                          0.0f, 25, 10, "Prueba", 10, 5);
    }
    juego._proyectiles.emplace_back(Posicion{110.0f, 110.0f}, 0.0f, 0.0f, 8.0f, 4.0f, 25);
    juego.actualizarProyectiles(0.01f);
    correcto &= comprobar(!juego._gestorOleadas.obtenerEnemigos()[0].estaVivo() &&
                          juego._gestorOleadas.obtenerEnemigos()[1].estaVivo() &&
                          juego._gestorOleadas.obtenerEnemigos()[2].estaVivo() && juego._proyectiles.empty(),
                          "un proyectil impacta solo al primer enemigo");
    correcto &= comprobar(juego.recursosSuelo.size() == 2,
                          "una recompensa por muerte");
    juego.actualizarProyectiles(0.01f);
    correcto &= comprobar(juego.recursosSuelo.size() == 2, "impacto eliminado no repite recompensa");

    juego._proyectiles.emplace_back(Posicion{110.0f, 110.0f}, 0.0f, 0.0f, 8.0f, 4.0f, 25);
    juego._proyectiles.emplace_back(Posicion{110.0f, 110.0f}, 0.0f, 0.0f, 8.0f, 4.0f, 25);
    juego.actualizarProyectiles(0.01f);
    correcto &= comprobar(!juego._gestorOleadas.obtenerEnemigos()[1].estaVivo() &&
                          !juego._gestorOleadas.obtenerEnemigos()[2].estaVivo() &&
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
    poblarEnemigos(juego);
    juego.actualizarDisparoAutomatico();
    correcto &= comprobar(juego._proyectiles.size() == 1, "disparo inmediato despues de reiniciar");

    juego._inventarioArmas.aplicarOferta({2, RarezaMejora::Comun, false});
    juego.seleccionarArmaPrueba(Juego::TipoArmaPrueba::Metralleta);
    correcto &= comprobar(juego.obtenerArmaSeleccionada().obtenerId() == 2 &&
                          juego.obtenerArmaSeleccionada().obtenerDano() == 6 &&
                          juego.obtenerArmaSeleccionada().obtenerVelocidadAtaque() == 8.0f &&
                          juego.obtenerArmaSeleccionada().obtenerAlcance() == 600.0f,
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
    correcto &= comprobar(juego._aspectoProyectiles.texturaDisponible(true) && juego._proyectiles.back().obtenerFormaColision().ancho == 14.0f,
                          "metralleta conserva textura disponible y dimensiones al elegir pistola");
    juego._inventarioArmas.aplicarOferta({2, RarezaMejora::Comun, false});
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
    correcto &= comprobar(juego._proyectiles.size() == 5 &&
                          juego._proyectiles[3].obtenerDano() == 25 &&
                          juego._proyectiles[3].obtenerFormaColision().ancho == 28.0f,
                          "pistola recupera cooldown estando inactiva y conserva su formato");
    juego.dibujar();
    correcto &= comprobar(juego._aspectoProyectiles.texturaDisponible(false) && juego._proyectiles.front().obtenerFormaColision().ancho == 28.0f,
                          "pistola conserva textura disponible y dimensiones despues de metralleta");
    juego._inventarioArmas.aplicarOferta({2, RarezaMejora::Comun, false});
    juego.seleccionarArmaPrueba(Juego::TipoArmaPrueba::Metralleta);
    juego.reiniciarPrueba();
    poblarEnemigos(juego);
    correcto &= comprobar(juego.obtenerArmaSeleccionada().obtenerId() == 1 &&
                          juego.armaPrueba.puedeDisparar() && juego._metralletaPrueba.puedeDisparar() &&
                          juego._proyectiles.empty(), "reinicio restaura ambas armas y selecciona pistola");

    for (Enemigo& enemigo : juego._gestorOleadas.obtenerEnemigos())
    {
        enemigo.recibirDanio(999);
    }
    juego._gestorOleadas.obtenerEnemigos()[0] = Enemigo(1, 100.0f, 100.0f, 56.0f, 56.0f,
                                    0.0f, 7, 10, "Prueba metralleta", 10, 5);
    juego.jugador.setPosicion({50.0f, 128.0f});
    juego._inventarioArmas.aplicarOferta({2, RarezaMejora::Comun, false});
    juego.seleccionarArmaPrueba(Juego::TipoArmaPrueba::Metralleta);
    juego.actualizarDisparoAutomatico();
    juego._proyectiles.erase(std::remove_if(juego._proyectiles.begin(), juego._proyectiles.end(),
        [](const Proyectil& proyectil) { return proyectil.obtenerIdArma() != 2; }), juego._proyectiles.end());
    const Posicion origenMetralleta = juego._proyectiles.front().obtenerPosicion();
    juego.actualizarProyectiles(0.075f);
    correcto &= comprobar(juego._gestorOleadas.obtenerEnemigos()[0].estaVivo() && juego._proyectiles.empty(),
                          "un disparo de seis no mata enemigo con siete de vida");
    juego.actualizarArmas(0.125f);
    juego.actualizarDisparoAutomatico();
    correcto &= comprobar(juego._proyectiles.front().obtenerPosicion().x == origenMetralleta.x,
                          "siguiente disparo nace en el mismo origen");
    juego.actualizarProyectiles(0.075f);
    correcto &= comprobar(!juego._gestorOleadas.obtenerEnemigos()[0].estaVivo() && juego.recursosSuelo.size() == 2,
                          "segundo disparo mata y entrega una recompensa");
    juego.reiniciarPrueba();
    poblarEnemigos(juego);
    for (Enemigo& enemigo : juego._gestorOleadas.obtenerEnemigos())
    {
        enemigo.recibirDanio(999);
    }
    juego.iniciarNuevaPartida();
    juego._gestorOleadas.obtenerEnemigos().push_back(crearEnemigo(BASICO, 1440.0f, 512.0f));
    for (float distancia : {599.0f, 600.0f, 601.0f})
    {
        juego._proyectiles.clear();
        juego.armaPrueba.reiniciar();
        juego._gestorOleadas.obtenerEnemigos()[0] = Enemigo(1, 960.0f + distancia - 28.0f,
            512.0f, 56.0f, 56.0f, 0.0f, 100, 10, "Rango", 10, 5);
        juego.actualizarDisparoAutomatico();
        correcto &= comprobar(juego._proyectiles.size() == (distancia <= 600.0f ? 1u : 0u),
                              "rango de adquisicion incluye 600 y excluye 601");
    }
    juego._proyectiles.clear();
    juego.armaPrueba.reiniciar();
    juego._gestorOleadas.obtenerEnemigos()[0] = Enemigo(1, 932.0f, -38.0f, 56.0f, 56.0f,
                                    0.0f, 100, 10, "Fuera de vista", 10, 5);
    juego.actualizarDisparoAutomatico();
    correcto &= comprobar(juego._proyectiles.empty(),
                          "enemigo dentro de rango pero fuera de pantalla no es objetivo");
    juego.jugador.setPosicion({7000.0f, 4000.0f});
    juego._escenario.actualizar(juego.jugador.getPosicion());
    juego._gestorOleadas.obtenerEnemigos()[0] = Enemigo(1, 7022.0f, 3972.0f, 56.0f, 56.0f,
                                    0.0f, 100, 10, "Vista desplazada", 10, 5);
    juego.actualizarDisparoAutomatico();
    correcto &= comprobar(juego._proyectiles.size() == 1,
                          "rango y visibilidad usan coordenadas del mundo");
    juego._proyectiles.clear();
    juego._gestorOleadas.obtenerEnemigos()[0] = Enemigo(1, 7030.0f, 3990.0f, 20.0f, 20.0f,
                                    0.0f, 10, 10, "No impactar al expirar", 10, 5);
    juego._proyectiles.emplace_back(Posicion{7000.0f, 4000.0f},
                                    100.0f, 0.0f, 8.0f, 4.0f, 25);
    juego._proyectiles.back().limitarAlcance(35.0f);
    juego.actualizarProyectiles(0.4f);
    correcto &= comprobar(juego._proyectiles.empty() && juego._gestorOleadas.obtenerEnemigos()[0].estaVivo(),
                          "proyectil agotado no aplica dano aunque termine sobre un enemigo");
    juego.reiniciarPrueba();
    correcto &= comprobar(juego._gestorOleadas.cantidadEnemigos() == 0 &&
                          juego._gestorOleadas.tiempoDePartida() == 0.0f,
                          "reinicio limpia oleadas y reloj");
    juego.actualizarEnemigos(1.5f);
    correcto &= comprobar(juego._gestorOleadas.cantidadEnemigos() == 1,
                          "primera aparicion desde Juego");
    Enemigo& enemigoOleada = juego._gestorOleadas.obtenerEnemigos().front();
    const Posicion posicionOleada = enemigoOleada.obtenerPosicion();
    juego._proyectiles.emplace_back(posicionOleada, 0.0f, 0.0f, 8.0f, 4.0f, 999);
    juego.actualizarProyectiles(0.01f);
    correcto &= comprobar(!enemigoOleada.estaVivo() && juego.recursosSuelo.size() == 2,
                          "proyectil mata enemigo generado y entrega recompensa");
    juego.actualizarEnemigos(0.01f);
    correcto &= comprobar(juego._gestorOleadas.cantidadEnemigos() == 0 &&
                          juego.recursosSuelo.size() == 2,
                          "limpieza de muertos conserva recompensa unica");
    for (int paso = 0; paso < 6000; ++paso)
    {
        juego.actualizarEnemigos(0.05f);
        correcto &= comprobar(juego._gestorOleadas.cantidadEnemigos() <=
                              juego._gestorOleadas.maximoVivosActual() + (juego._gestorOleadas.jefeEstaVivo() ? 1u : 0u),
                              "oleadas respetan techo de enemigos");
    }
    correcto &= comprobar(juego._gestorOleadas.maximoVivosActual() == 60 &&
                          std::abs(juego._gestorOleadas.intervaloActual() - (juego._gestorOleadas.jefeEstaVivo() ? 0.8f : 0.4f)) < 0.001f &&
                          juego._gestorOleadas.oleadaActual() == 11,
                          "dificultad maxima tras cinco minutos");
    juego.jugador.recibirDano(999);
    const float tiempoAlMorir = juego._gestorOleadas.tiempoDePartida();
    juego.actualizarEnemigos(1.0f);
    correcto &= comprobar(juego._gestorOleadas.tiempoDePartida() == tiempoAlMorir,
                          "jugador muerto detiene oleadas");
    juego.ventana.close();
    if (correcto)
    {
        std::cout << "Juego: multiples proyectiles, pruebas correctas\n";
    }
    return correcto ? 0 : 1;
}
