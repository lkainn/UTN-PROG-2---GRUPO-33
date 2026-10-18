#include <SFML/Graphics.hpp>
#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <random>
#include <string>
#include <vector>
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
    juego.iniciarNuevaPartida();
    juego._inventarioArmas.aplicarOferta({2, RarezaMejora::Rara, false});
    juego.aplicarBonificacionesArmas();
    auto& enemigos = juego._gestorOleadas.obtenerEnemigos();
    enemigos.push_back(crearEnemigo(BASICO, 1250.0f, 512.0f));
    juego.actualizarDisparoAutomatico();
    correcto &= comprobar(juego._proyectiles.size() == 2 &&
        juego._proyectiles[0].obtenerIdArma() == 1 && juego._proyectiles[1].obtenerIdArma() == 2,
        "los dos slots disparan en la misma actualizacion");
    juego.actualizarArmas(0.13f);
    juego.actualizarDisparoAutomatico();
    correcto &= comprobar(juego._proyectiles.size() == 3 &&
        juego._proyectiles.back().obtenerIdArma() == 2,
        "recargas independientes sin seleccionar el segundo slot");

    // Las bocas se verifican en las cuatro direcciones, con y sin texturas.
    AspectoArmas respaldo("recursos/no_existe");
    sf::RenderTexture muestra({1280, 1200});
    muestra.setView(sf::View(sf::FloatRect({0.0f, 0.0f}, {640.0f, 600.0f})));
    muestra.clear(sf::Color(18, 23, 29));
    for (int idArma = 1; idArma <= 4; ++idArma)
    {
        juego.iniciarNuevaPartida();
        juego._inventarioArmas._slots = {{idArma, 0}};
        for (int direccion = 0; direccion < 4; ++direccion)
        {
            const float angulo = direccion * 1.5707963268f;
            const Posicion centro = juego.jugador.getPosicion();
            enemigos.clear();
            enemigos.emplace_back(1, centro.x + std::cos(angulo) * 200.0f - 28.0f,
                centro.y + std::sin(angulo) * 200.0f - 28.0f,
                56.0f, 56.0f, 0.0f, 100, 10, "Objetivo", 5, 3);
            juego._proyectiles.clear();
            juego.obtenerArmaPorId(idArma).reiniciar();
            juego.actualizarDisparoAutomatico();
            correcto &= comprobar(!juego._proyectiles.empty(), "disparo en todas las direcciones");
            if (!juego._proyectiles.empty())
            {
                const Proyectil& proyectil = juego._proyectiles.front();
                const FormaColision forma = proyectil.obtenerFormaColision();
                const Posicion punta = juego._aspectoArmas.obtenerPunta(
                    juego.obtenerAnclajeArma(0), juego._angulosArmas[0], idArma);
                correcto &= comprobar(std::hypot(forma.x + forma.ancho / 2.0f - punta.x,
                    forma.y + forma.alto / 2.0f - punta.y) < 0.001f &&
                    std::hypot(punta.x - centro.x, punta.y - centro.y) > 30.0f,
                    "la bala nace en la boca y fuera del centro del jugador");
                const float desplazamientoX = punta.x - centro.x;
                const float desplazamientoY = punta.y - centro.y;
                correcto &= comprobar(std::abs(desplazamientoX * std::sin(angulo) -
                    desplazamientoY * std::cos(angulo)) < 0.001f &&
                    desplazamientoX * std::cos(angulo) + desplazamientoY * std::sin(angulo) > 30.0f,
                    "la boca queda delante del jugador sobre la linea al objetivo");
            }
            const Posicion puntaRespaldo = respaldo.obtenerPunta(centro, angulo, idArma);
            correcto &= comprobar(std::isfinite(puntaRespaldo.x) && std::isfinite(puntaRespaldo.y) &&
                std::hypot(puntaRespaldo.x - centro.x, puntaRespaldo.y - centro.y) > 30.0f,
                "boca valida sin textura y con giro");
            const Posicion anclaje{80.0f + direccion * 160.0f, 75.0f + (idArma - 1) * 150.0f};
            juego._aspectoArmas.dibujar(muestra, anclaje, angulo, idArma);
            const Posicion boca = juego._aspectoArmas.obtenerPunta(anclaje, angulo, idArma);
            sf::CircleShape marca(1.5f);
            marca.setOrigin({1.5f, 1.5f});
            marca.setPosition({boca.x, boca.y});
            marca.setFillColor(sf::Color::Red);
            muestra.draw(marca);
        }
    }
    muestra.display();
    std::filesystem::create_directories(".codex_tmp");
    correcto &= comprobar(muestra.getTexture().copyToImage().saveToFile(".codex_tmp/bocas_armas.png"),
        "muestra de las bocas en cuatro direcciones");

    juego.iniciarNuevaPartida();
    const Posicion centro = juego.jugador.getPosicion();
    juego.recursosSuelo.push_back({Juego::TipoRecurso::Chatarra, 20, centro});
    juego.actualizarRecursos(0.01f);
    juego.jugador.gastarChatarra(10);
    juego.jugador.ganarChatarra(10); // Simular un reembolso, que no es recoleccion.
    juego.actualizarPartida(1.0f);
    correcto &= comprobar(juego._partida.obtenerChatarraObtenida() == 20 &&
        juego._partida.obtenerPuntaje() == 40, "gastos y reembolsos no alteran lo recogido");
    enemigos.push_back(crearEnemigo(BASICO, 100.0f, 100.0f));
    enemigos.back().recibirDanio(9999);
    juego.actualizarPartida(0.5f);
    juego.actualizarEnemigos(0.01f);
    juego.actualizarPartida(0.5f);
    correcto &= comprobar(juego._partida.obtenerEnemigosEliminados() == 1,
        "baja contada una vez antes y despues de limpiar el vector");
    juego.manejarEscape();
    const int tiempoPausa = juego._partida.obtenerDuracionSegundos();
    juego.actualizar();
    correcto &= comprobar(juego._partida.obtenerDuracionSegundos() == tiempoPausa,
        "pausa no incrementa duracion");
    juego.manejarEscape();

    juego._gestorOleadas.actualizar(centro, 300.0f);
    for (Enemigo& enemigo : enemigos)
    {
        if (enemigo.obtenerIdEnemigo() == 4)
        {
            enemigo.recibirDanio(9999);
        }
    }
    juego.actualizarPartida(0.1f);
    correcto &= comprobar(juego._estado == Juego::EstadoJuego::Resultado &&
        juego._partida.fueVictoria() && juego._partida.obtenerEnemigosEliminados() == 2 &&
        juego._partida.obtenerPuntaje() == 5060,
        "muerte del jefe finaliza e incluye ultima baja y premio");
    const int tiempoFinal = juego._partida.obtenerDuracionSegundos();
    const float tiempoOleadas = juego._gestorOleadas.tiempoDePartida();
    juego.actualizar();
    correcto &= comprobar(juego._partida.obtenerDuracionSegundos() == tiempoFinal &&
        juego._gestorOleadas.tiempoDePartida() == tiempoOleadas,
        "resultado congela simulacion");
    std::filesystem::create_directories(".codex_tmp");
    juego.dibujar();
    juego.dibujar();
    sf::Texture captura(juego.ventana.getSize());
    captura.update(juego.ventana);
    correcto &= comprobar(captura.copyToImage().saveToFile(".codex_tmp/resultado_victoria.png"),
        "captura de resultado");
    juego.procesarEntradaMenu(sf::Keyboard::Scancode::Escape);
    correcto &= comprobar(juego._estado == Juego::EstadoJuego::Menu, "resultado vuelve al menu");
    juego.procesarEntradaMenu(sf::Keyboard::Scancode::Enter);
    correcto &= comprobar(juego._partida.estaEnCurso(), "jugar desde menu crea nueva partida");

    juego._inventarioArmas.aplicarOferta({3, RarezaMejora::Legendaria, true});
    juego.aplicarBonificacionesArmas();
    juego.jugador.ganarExperiencia(300);
    juego.jugador.aumentarVidaMaxima(200);
    juego.jugador.ganarChatarra(500);
    juego._costoCofre = 99;
    juego._estacionMejoras._costo = 99;
    juego._encuentros.generarAlrededor({12000.0f, 12000.0f});
    juego.jugador.recibirDano(9999);
    enemigos.push_back(crearEnemigo(JEFE, 100.0f, 100.0f));
    enemigos.back().recibirDanio(9999);
    juego.actualizarPartida(0.1f);
    correcto &= comprobar(juego._partida.obtenerResultado() == ResultadoPartida::Derrota,
        "derrota tiene prioridad si jefe y jugador mueren juntos");
    juego.procesarEntradaMenu(sf::Keyboard::Scancode::Enter);
    correcto &= comprobar(juego._estado == Juego::EstadoJuego::Jugando &&
        juego.jugador.getVidaMaxima() == 100 && juego.jugador.getVida() == 100 &&
        juego.jugador.getNivel() == 1 && juego.jugador.getExperiencia() == 0 &&
        juego.jugador.getChatarra() == 0 && juego._inventarioArmas.obtenerIdSlot(0) == 1 &&
        juego._inventarioArmas.obtenerIdSlot(1) == 0 && juego._costoCofre == 15 &&
        juego._estacionMejoras.obtenerCosto() == 10 && juego._proyectiles.empty() &&
        juego.recursosSuelo.empty() && juego._gestorOleadas.cantidadEnemigos() == 0 &&
        juego._partida.obtenerPuntaje() == 0 && juego.armaPrueba.obtenerDano() == 25 &&
        juego._escopetaPrueba.obtenerDano() == 12,
        "reintentar restablece jugador, armas, bonos, precios, oleadas y recursos");
    for (const auto& zona : juego._encuentros.obtenerZonas())
    {
        correcto &= comprobar(zona.first.first < 10 && zona.first.second < 10,
            "reinicio descarta zonas de la partida anterior");
        for (const ObjetoEncuentro& objeto : zona.second)
        {
            correcto &= comprobar(!objeto.usado, "encuentros nuevos sin consumir");
        }
    }
    juego.ventana.close();
    std::cout << (correcto ? "Partida integrada OK\n" : "Partida integrada FALLO\n");
    return correcto ? 0 : 1;
}
