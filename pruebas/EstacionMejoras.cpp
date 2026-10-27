#include <SFML/Graphics.hpp>
#include "EntornoDatosPrueba.h"
#include <array>
#include <cstddef>
#include <vector>
#include <iostream>
#include <limits>
#include <cmath>
#include <filesystem>
#include <map>
#include <random>
#include <string>
#include <utility>
#include "Arma.h"
#include "Jugador.h"
#include "Enemigo.h"
#include "EncuentroAleatorio.h"
#include "GestorOleadas.h"
#define private public
#include "EstacionMejoras.h"
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

bool mismasOfertas(const std::array<OfertaArma, 3>& antes, const EstacionMejoras& estacion)
{
    for (std::size_t indice = 0; indice < 3; ++indice)
    {
        const OfertaArma actual = estacion.obtenerOferta(indice);
        if (antes[indice].idArma != actual.idArma || antes[indice].rareza != actual.rareza ||
            antes[indice].especial != actual.especial)
        {
            return false;
        }
    }
    return true;
}

bool guardarVista(Juego& juego, const std::string& ruta)
{
    // Ambos buffers muestran la misma escena al capturar despues del intercambio.
    juego.dibujar();
    juego.dibujar();
    sf::Texture captura(juego.ventana.getSize());
    captura.update(juego.ventana);
    std::filesystem::create_directories(obtenerCarpetaSalidasPrueba());
    return captura.copyToImage().saveToFile(ruta);
}
}

int main()
{
    if (!prepararDatosPrueba())
    {
        return 1;
    }
    bool correcto = true;
    Juego juego;
    juego.ventana.setVisible(false);
    juego._generador.seed(314);
    juego._encuentros = EncuentroAleatorio(42);
    for (int indice = 0; indice < 5; ++indice)
    {
        juego._encuentros.generarAlrededor({indice * 6000.0f, 0.0f});
    }
    juego.seleccionarSlotArma(1);
    juego.seleccionarArmaPrueba(Juego::TipoArmaPrueba::Escopeta);
    correcto &= comprobar(juego.obtenerArmaSeleccionada().obtenerId() == 1,
        "slot vacio y arma no adquirida no se pueden seleccionar");
    irAEstacion(juego);
    const Posicion estacion = juego.jugador.getPosicion();
    juego.jugador.setPosicion({estacion.x + 101.0f, estacion.y});
    juego.jugador.ganarChatarra(500);
    juego.intentarComprarMejora();
    correcto &= comprobar(juego.jugador.getChatarra() == 500 &&
        !juego._estacionMejoras.menuAbierto(), "lejos no cobra");
    juego.jugador.setPosicion(estacion);
    juego.jugador.gastarChatarra(491);
    juego.intentarComprarMejora();
    correcto &= comprobar(juego.jugador.getChatarra() == 9 &&
        !juego._estacionMejoras.menuAbierto(), "saldo insuficiente no cobra");
    juego.jugador.ganarChatarra(491);
    juego.intentarComprarMejora();
    juego.intentarComprarMejora();
    correcto &= comprobar(juego.jugador.getChatarra() == 490 && juego._estacionMejoras.menuAbierto(),
        "cobra una vez al abrir");
    const auto ofertasIniciales = juego._estacionMejoras._ofertas;
    const auto estadoGenerador = juego._generador;
    juego._proyectiles.emplace_back(Posicion{500.0f, 400.0f}, 100.0f, 0.0f, 8.0f, 4.0f, 10);
    juego._gestorOleadas.obtenerEnemigos().push_back(crearEnemigo(BASICO, 100.0f, 100.0f));
    const float tiempoAntes = juego._gestorOleadas.tiempoDePartida();
    juego.actualizar();
    juego.actualizarDisparoAutomatico();
    correcto &= comprobar(juego._proyectiles.size() == 1 &&
        juego._proyectiles.front().obtenerPosicion().x == 500.0f &&
        juego._gestorOleadas.tiempoDePartida() == tiempoAntes, "menu pausa simulacion y disparos");
    juego.elegirMejoraEstacion(3);
    correcto &= comprobar(juego._estacionMejoras.menuAbierto() &&
        juego._estacionMejoras.obtenerCosto() == 10, "opcion invalida no consume");
    juego.cancelarCompraEstacion();
    juego.cancelarCompraEstacion();
    correcto &= comprobar(juego.jugador.getChatarra() == 500 && !juego._estacionMejoras.menuAbierto(),
        "cancelar reembolsa una vez sin reiniciar");
    juego.intentarComprarMejora();
    correcto &= comprobar(mismasOfertas(ofertasIniciales, juego._estacionMejoras) && juego._generador == estadoGenerador,
        "reabrir misma estacion no resortea");

    // Ofertas fijas para verificar cada transaccion sin depender de la semilla.
    juego._estacionMejoras._ofertas = {{{1, RarezaMejora::Comun, false},
        {2, RarezaMejora::Legendaria, true}, {3, RarezaMejora::Epica, true}}};
    correcto &= comprobar(guardarVista(juego, obtenerCarpetaSalidasPrueba() + "/estacion_ofertas.png"), "captura del menu de ofertas");
    juego.elegirMejoraEstacion(1);
    juego.elegirMejoraEstacion(1);
    correcto &= comprobar(juego._inventarioArmas.obtenerIdSlot(1) == 2 &&
        juego.obtenerArmaSeleccionada().obtenerId() == 2 && juego._metralletaPrueba.obtenerDano() == 8 &&
        juego._metralletaPrueba.obtenerCantidadProyectiles() == 2 &&
        std::abs(juego._metralletaPrueba.obtenerVelocidadAtaque() - 9.44f) < 0.001f &&
        juego._estacionMejoras.obtenerCosto() == 16 && juego.jugador.getVidaMaxima() == 100,
        "adquisicion legendaria incluye bonos solo del arma y fija slot2");
    juego.seleccionarSlotArma(0);
    correcto &= comprobar(juego.obtenerArmaSeleccionada().obtenerId() == 1, "tecla slot1 conserva pistola");
    juego.seleccionarSlotArma(1);
    correcto &= comprobar(juego.obtenerArmaSeleccionada().obtenerId() == 2, "tecla slot2 selecciona arma adquirida");

    irAEstacion(juego);
    juego.intentarComprarMejora();
    juego._estacionMejoras._ofertas[1] = {3, RarezaMejora::Epica, true};
    ObjetoEncuentro* reemplazo = juego._estacionSeleccionada;
    const auto ofertasReemplazo = juego._estacionMejoras._ofertas;
    juego.elegirMejoraEstacion(1);
    correcto &= comprobar(juego._opcionReemplazo == 1 && !reemplazo->usado &&
        juego._inventarioArmas.obtenerIdSlot(0) == 1 && juego._estacionMejoras.obtenerCosto() == 16,
        "reemplazo espera segunda confirmacion sin consumir estacion");
    correcto &= comprobar(guardarVista(juego, obtenerCarpetaSalidasPrueba() + "/estacion_reemplazo.png"), "captura advertencia irreversible");
    juego._opcionReemplazo = 3;
    correcto &= comprobar(mismasOfertas(ofertasReemplazo, juego._estacionMejoras) &&
        juego._inventarioArmas.obtenerIdSlot(0) == 1, "volver de confirmacion conserva ofertas y pistola");
    juego.elegirMejoraEstacion(1);
    juego.confirmarReemplazoPistola();
    juego.confirmarReemplazoPistola();
    correcto &= comprobar(juego._inventarioArmas.obtenerIdSlot(0) == 3 &&
        juego._inventarioArmas.obtenerIdSlot(1) == 2 && juego._escopetaPrueba.obtenerDano() == 15 &&
        juego._escopetaPrueba.obtenerCantidadProyectiles() == 6 && reemplazo->usado &&
        juego._estacionMejoras.obtenerCosto() == 26, "reemplazo fija slot1, preserva slot2 y aplica una sola mejora");
    juego.seleccionarArmaPrueba(Juego::TipoArmaPrueba::Pistola);
    correcto &= comprobar(juego.obtenerArmaSeleccionada().obtenerId() == 3, "pistola descartada no seleccionable");

    irAEstacion(juego);
    juego.intentarComprarMejora();
    for (std::size_t indice = 0; indice < 3; ++indice)
    {
        const int idArma = juego._estacionMejoras.obtenerOferta(indice).idArma;
        correcto &= comprobar(idArma == 2 || idArma == 3, "slots fijos solo ofrecen armas poseidas");
    }
    juego._estacionMejoras._ofertas[2] = {2, RarezaMejora::Rara, false};
    juego.elegirMejoraEstacion(2);
    correcto &= comprobar(juego._metralletaPrueba.obtenerDano() == 9 &&
        std::abs(juego._metralletaPrueba.obtenerVelocidadAtaque() - 10.08f) < 0.001f &&
        juego._estacionMejoras.obtenerCosto() == 42 && juego.jugador.getChatarra() == 448,
        "mejora arma poseida suma bonos y no ocupa otro slot");

    irAEstacion(juego);
    const Posicion pendiente = juego.jugador.getPosicion();
    juego.intentarComprarMejora();
    const auto ofertasPendientes = juego._estacionMejoras._ofertas;
    juego.reiniciarPrueba();
    correcto &= comprobar(juego.jugador.getChatarra() == 448 &&
        juego.obtenerArmaSeleccionada().obtenerId() == 3 && juego._inventarioArmas.obtenerIdSlot(1) == 2 &&
        juego._metralletaPrueba.obtenerDano() == 9 && juego._estacionMejoras.obtenerCosto() == 42 &&
        !juego._estacionMejoras.menuAbierto(), "R devuelve pago y conserva slots, bonos y precios sin restaurar pistola");
    juego.jugador.setPosicion(pendiente);
    juego.intentarComprarMejora();
    correcto &= comprobar(mismasOfertas(ofertasPendientes, juego._estacionMejoras), "R no permite resorteo");
    juego.cancelarCompraEstacion();
    juego.jugador.recibirDano(999);
    juego.intentarComprarMejora();
    correcto &= comprobar(!juego._estacionMejoras.menuAbierto() && juego.jugador.getChatarra() == 448,
        "muerto no compra");
    juego.ventana.close();
    std::cout << (correcto ? "Estacion: ofertas, slots, adquisicion, reemplazo, pausa y reembolso correctos\n" : "Hay fallos\n");
    return correcto ? 0 : 1;
}
