#include <SFML/Graphics.hpp>
#include "EntornoDatosPrueba.h"
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
#include "CarpetaDatos.h"

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

bool comprobarReapertura()
{
    using Tecla = sf::Keyboard::Scancode;
    Juego juego;
    juego.ventana.setVisible(false);
    juego.procesarEntradaMenu(Tecla::Enter);
    const auto& enemigos = juego._gestorOleadas.obtenerEnemigos();
    bool correcto = comprobar(juego._estado == Juego::EstadoJuego::Jugando &&
        juego.jugador.getPosicion().x == -1350.0f && juego.jugador.getPosicion().y == 2450.0f &&
        juego.jugador.getVida() == 83 && juego.jugador.getExperiencia() == 19 &&
        juego.jugador.getChatarra() == 150 && juego._gestorOleadas.tiempoDePartida() == 31.25f &&
        juego._partida.obtenerDuracionSegundos() == 31 && juego._partida.obtenerPuntaje() == 34 &&
        juego._inventarioArmas.obtenerIdSlot(1) == 3 &&
        juego._inventarioArmas.obtenerBonificaciones(3).especiales == 1 &&
        juego.recursosSuelo.size() == 1 && juego.recursosSuelo.front().cantidad == 9 &&
        !juego.armaPrueba.puedeDisparar() && juego.tiempoRestanteDanoContacto == 0.4f,
        "Continuar restaura la partida en otra ejecucion");
    correcto &= comprobar(enemigos.size() == 1 && juego._proyectiles.size() == 1 &&
        juego._proyectiles.front().yaImpacto(enemigos.front().obtenerIdInstancia()) &&
        juego._proyectiles.front().puedeRebotar() &&
        juego._proyectiles.front().obtenerDistanciaRestante() == 600.0f &&
        crearEnemigo(BASICO, 0.0f, 0.0f).obtenerIdInstancia() > enemigos.front().obtenerIdInstancia(),
        "enemigos y proyectiles conservan las referencias de impactos");
    correcto &= comprobar(!std::filesystem::exists(obtenerRutaDatos("PartidaEnCurso.dat")),
        "el guardado reanudado no se puede volver a cargar mientras se juega");
    juego.jugador.ganarExperiencia(juego.jugador.getExperienciaSiguienteNivel());
    juego.prepararCartas();
    juego._cartasDisponibles = {{true, true, true}};
    juego.salirDelJuego();
    return comprobar(!juego.ventana.isOpen(), "cerrar con cartas pendientes guarda la partida") && correcto;
}

bool comprobarGuardadoPendiente()
{
    using Tecla = sf::Keyboard::Scancode;
    Juego cartas;
    cartas.ventana.setVisible(false);
    cartas.procesarEntradaMenu(Tecla::Enter);
    bool correcto = comprobar(cartas._estado == Juego::EstadoJuego::SubiendoNivel &&
        cartas.jugador.requiereElegirMejora() && cartas._cartasDisponibles[2],
        "reanudar conserva la carta rara y la eleccion pendiente");
    cartas.procesarEntradaMenu(Tecla::Num1);
    correcto &= comprobar(cartas._estado == Juego::EstadoJuego::Jugando,
        "se puede elegir la carta restaurada");
    for (const auto& zona : cartas._encuentros.obtenerZonas())
    {
        const ObjetoEncuentro& objeto = zona.second[0];
        if (objeto.presente && !objeto.usado)
        {
            cartas.jugador.setPosicion(objeto.posicion);
            break;
        }
    }
    const int saldoAntes = cartas.jugador.getChatarra();
    cartas.intentarComprarMejora();
    if (!comprobar(cartas._estacionMejoras.menuAbierto(), "abrir estacion antes de guardar"))
    {
        return false;
    }
    const OfertaArma oferta = cartas._estacionMejoras.obtenerOferta(0);
    cartas.salirDelJuego();
    correcto &= comprobar(!cartas.ventana.isOpen(), "cerrar durante una compra guarda la partida");

    Juego estacion;
    estacion.ventana.setVisible(false);
    estacion.procesarEntradaMenu(Tecla::Enter);
    const OfertaArma ofertaCargada = estacion._estacionMejoras.obtenerOferta(0);
    correcto &= comprobar(estacion._estado == Juego::EstadoJuego::Jugando &&
        estacion._estacionSeleccionada != nullptr && estacion._estacionMejoras.menuAbierto() &&
        estacion.jugador.getChatarra() == saldoAntes - estacion._estacionMejoras.obtenerCosto() &&
        oferta.idArma == ofertaCargada.idArma && oferta.rareza == ofertaCargada.rareza &&
        oferta.especial == ofertaCargada.especial, "reanudar conserva la compra y sus ofertas");
    estacion.manejarEscape();
    correcto &= comprobar(estacion.jugador.getChatarra() == saldoAntes &&
        !estacion._estacionMejoras.menuAbierto(), "cancelar la compra restaurada reembolsa una sola vez");
    if (!comprobar(estacion.guardarPartida(), "guardar otra vez despues de reanudar"))
    {
        return false;
    }
    estacion.ventana.close();

    const std::string ruta = obtenerRutaDatos("PartidaEnCurso.dat");
    std::filesystem::resize_file(ruta, 12);
    Juego daniada;
    daniada.ventana.setVisible(false);
    daniada.procesarEntradaMenu(Tecla::Enter);
    correcto &= comprobar(daniada._estado == Juego::EstadoJuego::Menu &&
        daniada._gestorOleadas.tiempoDePartida() == 0.0f &&
        std::filesystem::file_size(ruta) == 12, "guardado truncado se rechaza sin aplicarlo");
    daniada.procesarEntradaMenu(Tecla::Down);
    daniada.procesarEntradaMenu(Tecla::Enter);
    daniada.procesarEntradaMenu(Tecla::Escape);
    correcto &= comprobar(std::filesystem::exists(ruta), "cancelar Nueva partida conserva el guardado");
    daniada.procesarEntradaMenu(Tecla::Enter);
    daniada.procesarEntradaMenu(Tecla::Enter);
    correcto &= comprobar(daniada._estado == Juego::EstadoJuego::Jugando &&
        !std::filesystem::exists(ruta) && daniada.jugador.getNivel() == 1 &&
        daniada.jugador.getChatarra() == 0, "confirmar Nueva partida descarta el guardado y reinicia");
    std::filesystem::create_directory(ruta);
    daniada.salirDelJuego();
    correcto &= comprobar(daniada.ventana.isOpen() && daniada._estado == Juego::EstadoJuego::Pausa &&
        std::filesystem::is_directory(ruta), "si falla el guardado se puede volver a intentar");
    std::filesystem::remove(ruta);
    return correcto;
}
}

int main(int argc, char* argv[])
{
    if (argc == 2)
    {
        const char* carpetaPrueba = std::getenv("TPI_CARPETA_DATOS");
        if (carpetaPrueba != nullptr && carpetaPrueba[0] != '\0')
        {
            if (std::string(argv[1]) == "--reanudar")
            {
                return comprobarReapertura() ? 0 : 1;
            }
            if (std::string(argv[1]) == "--verificar-pendiente")
            {
                const bool correcto = comprobarGuardadoPendiente();
                std::cout << (correcto ? "Menu principal y reapertura OK\n" : "Menu principal FALLO\n");
                return correcto ? 0 : 1;
            }
        }
        return 1;
    }
    if (!comprobar(prepararDatosPrueba(), "preparar archivos aislados de prueba"))
    {
        return 1;
    }
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
                          obtenerCarpetaSalidasPrueba() + "/menu_principal.png"), "captura del menu");

    juego.procesarEntradaMenu(sf::Keyboard::Scancode::Up);
    correcto &= comprobar(juego._opcionMenu == 4, "navegacion circular hacia arriba");
    juego.procesarEntradaMenu(sf::Keyboard::Scancode::Down);
    juego.procesarEntradaMenu(sf::Keyboard::Scancode::Enter);
    correcto &= comprobar(juego._estado == Juego::EstadoJuego::Menu && !juego._continuacionDisponible,
        "Continuar queda en el menu cuando no hay guardado");
    juego.procesarEntradaMenu(sf::Keyboard::Scancode::Down);
    juego.procesarEntradaMenu(sf::Keyboard::Scancode::Enter);
    correcto &= comprobar(juego._estado == Juego::EstadoJuego::ConfirmarReinicio,
        "Nueva partida pide confirmacion");
    juego.procesarEntradaMenu(sf::Keyboard::Scancode::Escape);
    correcto &= comprobar(juego._estado == Juego::EstadoJuego::Menu,
        "Escape cancela el reinicio");
    juego.procesarEntradaMenu(sf::Keyboard::Scancode::Down);
    juego.procesarEntradaMenu(sf::Keyboard::Scancode::Enter);
    juego.procesarEntradaMenu(sf::Keyboard::Scancode::Down);
    juego.procesarEntradaMenu(sf::Keyboard::Scancode::Escape);
    correcto &= comprobar(juego.ventana.isOpen() && juego._opcionMenu == 2 &&
                          juego._estado == Juego::EstadoJuego::Menu,
                          "controles vuelve sin cerrar ni cambiar seleccion");
    juego.procesarEntradaMenu(sf::Keyboard::Scancode::Enter);
    juego.dibujar();
    juego.dibujar();
    captura.update(juego.ventana);
    correcto &= comprobar(captura.copyToImage().saveToFile(
                          obtenerCarpetaSalidasPrueba() + "/menu_controles.png"), "captura de controles");
    juego.procesarEntradaMenu(sf::Keyboard::Scancode::Enter);
    juego.procesarEntradaMenu(sf::Keyboard::Scancode::Down);
    juego.procesarEntradaMenu(sf::Keyboard::Scancode::Enter);
    correcto &= comprobar(juego._estado == Juego::EstadoJuego::Ranking && juego._ranking.empty(),
                          "ranking abre sin historial");
    const float tiempoRanking = juego._gestorOleadas.tiempoDePartida();
    juego.actualizar();
    correcto &= comprobar(juego._gestorOleadas.tiempoDePartida() == tiempoRanking &&
                          juego.jugador.getPosicion().x == posicion.x &&
                          juego.jugador.getPosicion().y == posicion.y,
                          "ranking no avanza combate");
    juego.dibujar();
    juego.dibujar();
    captura.update(juego.ventana);
    correcto &= comprobar(captura.copyToImage().saveToFile(
                          obtenerCarpetaSalidasPrueba() + "/menu_ranking.png"), "captura del ranking");
    juego.procesarEntradaMenu(sf::Keyboard::Scancode::Escape);
    correcto &= comprobar(juego._estado == Juego::EstadoJuego::Menu && juego._opcionMenu == 3,
                          "Escape vuelve a la opcion ranking");
    juego.procesarEntradaMenu(sf::Keyboard::Scancode::Enter);
    juego.procesarEntradaMenu(sf::Keyboard::Scancode::Enter);
    correcto &= comprobar(juego._estado == Juego::EstadoJuego::Menu && juego._opcionMenu == 3,
                          "Enter vuelve del ranking sin iniciar partida");
    juego.procesarEntradaMenu(sf::Keyboard::Scancode::Up);
    juego.procesarEntradaMenu(sf::Keyboard::Scancode::Up);
    sf::sleep(sf::milliseconds(100));
    juego.procesarEntradaMenu(sf::Keyboard::Scancode::Enter);
    juego.procesarEntradaMenu(sf::Keyboard::Scancode::Enter);
    correcto &= comprobar(juego._estado == Juego::EstadoJuego::Jugando &&
                          juego.relojFotogramas.getElapsedTime().asSeconds() < 0.05f,
                          "Nueva partida descarta espera del menu");
    juego.actualizar();
    juego.dibujar();
    juego.dibujar();

    juego.jugador.ganarExperiencia(juego.jugador.getExperienciaSiguienteNivel());
    juego.actualizar();
    const float tiempoCartas = juego._gestorOleadas.tiempoDePartida();
    juego.actualizar();
    juego.dibujar();
    correcto &= comprobar(juego._estado == Juego::EstadoJuego::SubiendoNivel &&
        juego._gestorOleadas.tiempoDePartida() == tiempoCartas, "elegir carta pausa la partida");
    juego.procesarEntradaMenu(sf::Keyboard::Scancode::Num1);
    correcto &= comprobar(juego._estado == Juego::EstadoJuego::Jugando &&
        !juego.jugador.requiereElegirMejora(), "seleccionar carta reanuda la partida");

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
                          obtenerCarpetaSalidasPrueba() + "/menu_pausa.png"), "captura de pausa");
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

    juego._encuentros.generarAlrededor({6000.0f, 6000.0f});
    juego._encuentros.generarAlrededor({-6000.0f, -6000.0f});
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

    using Tecla = sf::Keyboard::Scancode;
    juego.jugador.cargarEstado(83, 2, 19, 150);
    juego.jugador.setPosicion({-1350.0f, 2450.0f});
    juego._gestorOleadas.reiniciar();
    juego._gestorOleadas.actualizar(juego.jugador.getPosicion(), 31.25f);
    juego._partida = Partida(1);
    juego._partida.actualizar(31.25f);
    juego._partida.registrarChatarra(17);
    juego._inventarioArmas.aplicarOferta({3, RarezaMejora::Legendaria, true});
    juego.aplicarBonificacionesArmas();
    // Con dos impactos totales, un impacto queda registrado y el proyectil sigue en vuelo.
    juego._proyectiles.front() = Proyectil({500.0f, 400.0f}, 100.0f, 0.0f, 8.0f, 4.0f, 10, 1, 2);
    juego._proyectiles.front().limitarAlcance(600.0f);
    juego._proyectiles.front().configurarRebotes(2);
    juego._proyectiles.front().registrarImpacto(
        juego._gestorOleadas.obtenerEnemigos().front().obtenerIdInstancia());
    juego.recursosSuelo = {{Juego::TipoRecurso::Experiencia, 9, {110.0f, 120.0f}}};
    juego.manejarEscape();
    juego.procesarEntradaMenu(Tecla::Down);
    juego.procesarEntradaMenu(Tecla::Enter);
    correcto &= comprobar(!juego.ventana.isOpen() &&
        std::filesystem::is_regular_file(obtenerRutaDatos("PartidaEnCurso.dat")),
        "Salir en pausa guarda antes de cerrar");

    return correcto ? 0 : 1;
}
