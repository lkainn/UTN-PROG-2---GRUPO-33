#include <SFML/Graphics.hpp>
#include "EntornoDatosPrueba.h"
#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include "EncuentroAleatorio.h"
#include "ArchivoJugadores.h"
#include "ArchivoMejoras.h"
#include "ArchivoPartidas.h"
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
    if (!prepararDatosPrueba())
    {
        return 1;
    }
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
    for (std::size_t slot = 0; slot < juego._proyectiles.size() && slot < 2; ++slot)
    {
        const Posicion mano = juego._aspectoJugador.obtenerMano(juego.jugador.getPosicion(), slot);
        const Posicion anclaje = juego.obtenerAnclajeArma(slot);
        const Posicion boca = juego._aspectoArmas.obtenerPuntaEnMano(mano,
            juego._angulosArmas[slot], juego._inventarioArmas.obtenerIdSlot(slot), juego._armasReflejadas[slot]);
        const FormaColision bala = juego._proyectiles[slot].obtenerFormaColision();
        correcto &= comprobar(std::hypot(anclaje.x - mano.x, anclaje.y - mano.y) < 0.001f &&
            std::hypot(bala.x + bala.ancho / 2.0f - boca.x, bala.y + bala.alto / 2.0f - boca.y) < 0.001f,
            "cada slot dispara desde el arma de su propia mano");
    }
    juego.actualizarArmas(0.13f);
    juego.actualizarDisparoAutomatico();
    correcto &= comprobar(juego._proyectiles.size() == 3 &&
        juego._proyectiles.back().obtenerIdArma() == 2,
        "recargas independientes sin seleccionar el segundo slot");

    const std::size_t disparosAntes = juego._proyectiles.size();
    const Posicion centroApuntado = juego.jugador.getPosicion();
    enemigos.clear();
    enemigos.emplace_back(1, centroApuntado.x - 28, centroApuntado.y - 228,
        56, 56, 0, 100, 10, "Objetivo", 5, 3);
    juego.actualizarDisparoAutomatico();
    const Posicion vista = juego._aspectoJugador.obtenerDireccionVista();
    correcto &= comprobar(std::abs(vista.x) < 0.001f && vista.y < -0.99f &&
        juego._proyectiles.size() == disparosAntes, "rig orientado hacia arriba aunque ambas armas esten recargando");
    for (std::size_t slot = 0; slot < 2; ++slot)
    {
        const Posicion boca = juego._aspectoArmas.obtenerPuntaEnMano(juego.obtenerAnclajeArma(slot),
            juego._angulosArmas[slot], juego._inventarioArmas.obtenerIdSlot(slot), juego._armasReflejadas[slot]);
        const float x = centroApuntado.x - boca.x;
        const float y = centroApuntado.y - 200 - boca.y;
        const float angulo = juego._angulosArmas[slot];
        correcto &= comprobar(std::abs(x * std::sin(angulo) - y * std::cos(angulo)) < 0.002f,
            "ambas armas siguen el objetivo con los anclajes de la nueva pose durante recarga");
    }

    juego.iniciarNuevaPartida();
    juego._inventarioArmas._slots = {{3, 2}};
    enemigos.clear();
    enemigos.emplace_back(1, centroApuntado.x + 472, centroApuntado.y - 28,
        56, 56, 0, 100, 10, "Objetivo", 5, 3);
    juego.actualizarDisparoAutomatico();
    correcto &= comprobar(juego._proyectiles.size() == 1 && juego._proyectiles[0].obtenerIdArma() == 2,
        "el arma corta apunta pero no dispara fuera de su alcance");
    for (std::size_t slot = 0; slot < 2; ++slot)
    {
        const Posicion boca = juego._aspectoArmas.obtenerPuntaEnMano(juego.obtenerAnclajeArma(slot),
            juego._angulosArmas[slot], juego._inventarioArmas.obtenerIdSlot(slot), juego._armasReflejadas[slot]);
        const float x = centroApuntado.x + 500 - boca.x;
        const float y = centroApuntado.y - boca.y;
        const float angulo = juego._angulosArmas[slot];
        correcto &= comprobar(std::abs(x * std::sin(angulo) - y * std::cos(angulo)) < 0.003f,
            "las dos manos apuntan al objetivo comun incluso con alcances distintos");
    }
    enemigos.clear();
    const Posicion desplazada{centroApuntado.x - 5, centroApuntado.y};
    juego._aspectoJugador.reiniciar(centroApuntado);
    juego.jugador.setPosicion(desplazada);
    juego.jugador.actualizar(false);
    juego._aspectoJugador.actualizar(desplazada);
    juego.actualizarDisparoAutomatico();
    correcto &= comprobar(juego._aspectoJugador.obtenerDireccionVista().x < -0.99f &&
        std::cos(juego._angulosArmas[0]) < -0.99f && std::cos(juego._angulosArmas[1]) < -0.99f,
        "sin enemigos, el cuerpo y ambas armas siguen la direccion de movimiento");

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
                const Posicion mano = juego.obtenerAnclajeArma(0);
                const Posicion punta = juego._aspectoArmas.obtenerPuntaEnMano(
                    mano, juego._angulosArmas[0], idArma, juego._armasReflejadas[0]);
                correcto &= comprobar(std::hypot(forma.x + forma.ancho / 2.0f - punta.x,
                    forma.y + forma.alto / 2.0f - punta.y) < 0.001f &&
                    std::hypot(punta.x - mano.x, punta.y - mano.y) > 10.0f,
                    "la bala nace en la boca del arma sostenida");
                const float haciaObjetivoX = centro.x + std::cos(angulo) * 200.0f - punta.x;
                const float haciaObjetivoY = centro.y + std::sin(angulo) * 200.0f - punta.y;
                const float eje = juego._angulosArmas[0];
                correcto &= comprobar(std::abs(haciaObjetivoX * std::sin(eje) -
                    haciaObjetivoY * std::cos(eje)) < 0.001f &&
                    haciaObjetivoX * std::cos(eje) + haciaObjetivoY * std::sin(eje) > 0.0f,
                    "el rayo desde la boca coincide con el objetivo al apuntar desde la mano");
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
    std::filesystem::create_directories(obtenerCarpetaSalidasPrueba());
    correcto &= comprobar(muestra.getTexture().copyToImage().saveToFile(obtenerCarpetaSalidasPrueba() + "/bocas_armas.png"),
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
    ArchivoPartidas historial;
    correcto &= comprobar(juego._partidaGuardada && historial.cantidadRegistros() == 1,
        "victoria guardada una vez en el historial");
    juego.actualizarPartida(0.1f);
    correcto &= comprobar(historial.cantidadRegistros() == 1, "resultado repetido no duplica guardado");
    const int tiempoFinal = juego._partida.obtenerDuracionSegundos();
    const float tiempoOleadas = juego._gestorOleadas.tiempoDePartida();
    juego.actualizar();
    correcto &= comprobar(juego._partida.obtenerDuracionSegundos() == tiempoFinal &&
        juego._gestorOleadas.tiempoDePartida() == tiempoOleadas,
        "resultado congela simulacion");
    std::filesystem::create_directories(obtenerCarpetaSalidasPrueba());
    juego.dibujar();
    juego.dibujar();
    sf::Texture captura(juego.ventana.getSize());
    captura.update(juego.ventana);
    correcto &= comprobar(captura.copyToImage().saveToFile(obtenerCarpetaSalidasPrueba() + "/resultado_victoria.png"),
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
    ArchivoJugadores archivoPerfil;
    const RegistroJugador perfilGuardado = archivoPerfil.leer();
    const int vidaPerfil = 100 + (perfilGuardado.nivelMaximo - 1) * 20;
    correcto &= comprobar(historial.cantidadRegistros() == 2 && perfilGuardado.puntajeMaximo == 5060,
        "derrota agrega historial y conserva el mejor puntaje anterior");
    juego.procesarEntradaMenu(sf::Keyboard::Scancode::Enter);
    correcto &= comprobar(juego._estado == Juego::EstadoJuego::Jugando &&
        juego.jugador.getVidaMaxima() == vidaPerfil && juego.jugador.getVida() == vidaPerfil &&
        juego.jugador.getNivel() == perfilGuardado.nivelMaximo && juego.jugador.getExperiencia() == 0 &&
        juego.jugador.getChatarra() == perfilGuardado.chatarraTotal && juego._inventarioArmas.obtenerIdSlot(0) == 1 &&
        juego._inventarioArmas.obtenerIdSlot(1) == 0 && juego._costoCofre == 15 &&
        juego._estacionMejoras.obtenerCosto() == 10 && juego._proyectiles.empty() &&
        juego.recursosSuelo.empty() && juego._gestorOleadas.cantidadEnemigos() == 0 &&
        juego._partida.obtenerPuntaje() == 0 && juego.armaPrueba.obtenerDano() == 25 &&
        juego._escopetaPrueba.obtenerDano() == 12,
        "reintentar recupera perfil y restablece armas, bonos temporales, precios, oleadas y recursos");
    for (const auto& zona : juego._encuentros.obtenerZonas())
    {
        correcto &= comprobar(zona.first.first < 10 && zona.first.second < 10,
            "reinicio descarta zonas de la partida anterior");
        for (const ObjetoEncuentro& objeto : zona.second)
        {
            correcto &= comprobar(!objeto.usado, "encuentros nuevos sin consumir");
        }
    }
    const int saldoInicial = juego.jugador.getChatarra();
    correcto &= comprobar(juego.comprarMejoraPermanente() && juego.jugador.getChatarra() == saldoInicial - 10 &&
        juego.jugador.getVidaMaxima() == vidaPerfil + 20, "compra permanente cobra una vez y aplica vida");
    juego.iniciarNuevaPartida();
    correcto &= comprobar(juego.jugador.getVidaMaxima() == vidaPerfil + 20 &&
        juego.jugador.getChatarra() == saldoInicial - 10, "perfil y mejora sobreviven a una nueva partida");
    for (int nivel = 1; nivel < 5; ++nivel)
    {
        correcto &= comprobar(juego.comprarMejoraPermanente(), "comprar hasta el limite del catalogo");
    }
    const int saldoAlTope = juego.jugador.getChatarra();
    correcto &= comprobar(!juego.comprarMejoraPermanente() && juego.jugador.getChatarra() == saldoAlTope,
        "mejora al maximo no cobra chatarra");
    correcto &= comprobar(juego.reiniciarProgreso() && juego.jugador.getNivel() == 1 &&
        juego.jugador.getChatarra() == 0 && juego.jugador.getVidaMaxima() == 100 &&
        juego._inventarioArmas.obtenerIdSlot(1) == 0 && historial.cantidadRegistros() == 2 &&
        juego._partida.estaEnCurso(), "reiniciar progreso restaura todo el juego sin borrar historial");
    correcto &= comprobar(!juego.comprarMejoraPermanente() && juego.jugador.getChatarra() == 0,
        "compra sin saldo no altera el perfil");
    juego.ventana.close();
    std::cout << (correcto ? "Partida integrada OK\n" : "Partida integrada FALLO\n");
    return correcto ? 0 : 1;
}
