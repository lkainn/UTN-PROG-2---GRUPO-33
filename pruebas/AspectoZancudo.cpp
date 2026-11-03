#include "EntornoDatosPrueba.h"
#include "Enemigo.h"
#include "SistemaColisiones.h"
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <unordered_map>
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

sf::Image representar(const AspectoEnemigos& aspecto, const Enemigo& enemigo)
{
    sf::RenderTexture superficie({256, 256});
    superficie.clear(sf::Color::Transparent);
    aspecto.dibujar(superficie, enemigo);
    superficie.display();
    return superficie.getTexture().copyToImage();
}

bool iguales(const sf::Image& a, const sf::Image& b)
{
    const auto tamanio = a.getSize();
    return b.getSize() == tamanio && std::equal(a.getPixelsPtr(),
        a.getPixelsPtr()+tamanio.x*tamanio.y*4, b.getPixelsPtr());
}
}

int main()
{
    if (!prepararDatosPrueba())
    {
        return 1;
    }
    bool correcto = true;
    const std::filesystem::path raiz = std::filesystem::exists("ProyectoTPI.cbp") ? "." : "../..";
    const auto salida = raiz / "bin/Debug/zancudo_revision";
    std::filesystem::create_directories(salida);
    const auto carpetaMordaza = (raiz / "recursos/enemigos/mordaza_atlas").string();
    const auto carpetaZancudo = (raiz / "recursos/enemigos/zancudo_atlas").string();
    const auto ausente = (salida / "ausente").string();
    AspectoEnemigos aspecto;
    if (!comprobar(aspecto.animacionDisponible(1) && aspecto.animacionDisponible(2),
                   "ambos paquetes cargados"))
    {
        return 1;
    }
    correcto &= comprobar(!aspecto.animacionDisponible(0) && !aspecto.animacionDisponible(5) &&
                          !aspecto.animacionDisponible(-1), "consulta de variante desconocida segura");
    for (const auto& paquete : aspecto._paquetes)
    {
        for (const sf::Texture& textura : paquete.texturas)
        {
            correcto &= comprobar(!textura.isSmooth(), "ambos atlas sin suavizado");
        }
    }
    {
        AspectoEnemigos soloMordaza(carpetaMordaza.c_str(), ausente.c_str());
        AspectoEnemigos soloZancudo(ausente.c_str(), carpetaZancudo.c_str());
        correcto &= comprobar(soloMordaza.animacionDisponible(1) &&
                              !soloMordaza.animacionDisponible(2), "Zancudo ausente no desactiva Mordaza");
        correcto &= comprobar(!soloZancudo.animacionDisponible(1) &&
                              soloZancudo.animacionDisponible(2), "Mordaza ausente no desactiva Zancudo");
        std::vector<Enemigo> mezcla{crearEnemigo(BASICO, 100, 100), crearEnemigo(RAPIDO, 100, 100)};
        soloMordaza.actualizar(mezcla, {1000, 1000, 48, 48}, 0.025f);
        soloZancudo.actualizar(mezcla, {1000, 1000, 48, 48}, 0.025f);
        correcto &= comprobar(soloMordaza._estados.size() == 1 && soloZancudo._estados.size() == 1,
                              "solo se guardan estados de recursos disponibles");
        sf::RenderTexture superficie({256, 256});
        correcto &= comprobar(!soloMordaza.dibujar(superficie, mezcla[1]) &&
                              !soloZancudo.dibujar(superficie, mezcla[0]),
                              "el respaldo se decide por variante");
    }
    {
        AspectoEnemigos intercambiado(carpetaMordaza.c_str(), carpetaMordaza.c_str());
        correcto &= comprobar(intercambiado.animacionDisponible(1) &&
                              !intercambiado.animacionDisponible(2), "cabecera de otra variante se rechaza");
    }
    std::vector<Enemigo> enemigos{crearEnemigo(BASICO, 100, 100), crearEnemigo(RAPIDO, 100, 100)};
    const auto idMordaza = enemigos[0].obtenerIdInstancia();
    const auto idZancudo = enemigos[1].obtenerIdInstancia();
    const FormaColision lejos{1000, 1000, 48, 48};
    aspecto.actualizar(enemigos, lejos, 0.025f);
    const auto reposo = representar(aspecto, enemigos[1]);
    enemigos[0].actualizar({1000, 100}, 0.025f);
    enemigos[1].actualizar({1000, 100}, 0.025f);
    aspecto.actualizar(enemigos, lejos, 0.025f);
    correcto &= comprobar(aspecto._estados.size() == 2 &&
                          aspecto._estados.at(idMordaza).fase > 0 &&
                          aspecto._estados.at(idZancudo).fase > 0, "horda mixta conserva fases independientes");
    correcto &= comprobar(std::abs(enemigos[1].obtenerPosicion().x-105.5f) < 0.001f &&
                          enemigos[1].obtenerFormaColision().ancho == 40 &&
                          enemigos[1].obtenerFormaColision().alto == 40 &&
                          enemigos[1].obtenerDano() == 8,
                          "se conservan velocidad 220, colision 40 y dano 8");
    correcto &= comprobar(!iguales(reposo, representar(aspecto, enemigos[1])), "carrera cambia pose");
    float fase = aspecto._estados.at(idZancudo).fase;
    aspecto.actualizar(enemigos, lejos, 0.025f);
    correcto &= comprobar(!aspecto._estados.at(idZancudo).enMovimiento &&
                          aspecto._estados.at(idZancudo).fase == fase, "reposo conserva fase");
    const std::array<Posicion, 8> movimientos{{{0, 1}, {1, 1}, {1, 0}, {1, -1},
                                            {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}}};
    for (int direccion = 0; direccion < 8; ++direccion)
    {
        const Posicion p = enemigos[1].obtenerPosicion();
        enemigos[1].actualizar({p.x+movimientos[direccion].x*100,
                               p.y+movimientos[direccion].y*100}, 0.025f);
        aspecto.actualizar(enemigos, lejos, 0.025f);
        correcto &= comprobar(aspecto._estados.at(idZancudo).direccion == direccion,
                              "Zancudo gira en las ocho direcciones");
    }
    const auto congelado = representar(aspecto, enemigos[1]);
    aspecto.actualizar(enemigos, lejos, 0);
    aspecto.actualizar(enemigos, lejos, std::numeric_limits<float>::quiet_NaN());
    correcto &= comprobar(iguales(congelado, representar(aspecto, enemigos[1])), "pausa no cambia el cuadro");
    const auto forma = enemigos[1].obtenerFormaColision();
    aspecto.actualizar(enemigos, forma, 0.025f);
    const auto contacto = representar(aspecto, enemigos[1]);
    aspecto.actualizar(enemigos, forma, 0.075f);
    correcto &= comprobar(!iguales(contacto, representar(aspecto, enemigos[1])),
                          "contacto comprime paragolpes y cuerpo");
    correcto &= comprobar(enemigos[1].estaVivo(), "reaccion visual no aplica dano");
    enemigos[1].recibirDanio(1000);
    aspecto.actualizar(enemigos, lejos, 0.025f);
    correcto &= comprobar(aspecto._estados.count(idZancudo) == 0 &&
                          aspecto._estados.count(idMordaza) == 1, "muerte limpia solo la instancia eliminada");
    enemigos[1] = crearEnemigo(RAPIDO, 100, 100);
    aspecto.actualizar(enemigos, lejos, 0.025f);
    correcto &= comprobar(aspecto._estados.at(enemigos[1].obtenerIdInstancia()).fase == 0,
                          "nueva instancia no hereda fase de la anterior");
    aspecto.reiniciar();
    correcto &= comprobar(aspecto._estados.empty(), "reinicio vacia las dos variantes");
    // Mismo desplazamiento repartido en distinta cantidad de fotogramas.
    {
        AspectoEnemigos dividido;
        std::vector<Enemigo> pasosA{crearEnemigo(RAPIDO, 100, 100)};
        std::vector<Enemigo> pasosB = pasosA;
        const auto id = pasosA[0].obtenerIdInstancia();
        aspecto.actualizar(pasosA, lejos, 0.01f);
        dividido.actualizar(pasosB, lejos, 0.01f);
        for (int i = 0; i < 40; ++i)
        {
            pasosA[0].actualizar({2000, 100}, 0.01f);
            aspecto.actualizar(pasosA, lejos, 0.01f);
        }
        for (int i = 0; i < 20; ++i)
        {
            pasosB[0].actualizar({2000, 100}, 0.02f);
            dividido.actualizar(pasosB, lejos, 0.02f);
        }
        correcto &= comprobar(std::abs(aspecto._estados.at(id).fase-dividido._estados.at(id).fase) < 0.0001f,
                              "fase independiente de la cantidad de fotogramas");
    }
    const auto invalido = salida / ("atlas_invalido_"+
        std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::filesystem::create_directories(invalido);
    std::filesystem::copy_file(std::filesystem::path(carpetaZancudo)/"animacion.txt", invalido/"animacion.txt");
    sf::Image truncada({767, 1344}, sf::Color::White);
    if (!truncada.saveToFile(invalido/"cuerpo_0.png"))
    {
        return 1;
    }
    {
        AspectoEnemigos dimensiones(carpetaMordaza.c_str(), invalido.string().c_str());
        correcto &= comprobar(dimensiones.animacionDisponible(1) && !dimensiones.animacionDisponible(2),
                              "textura truncada rechaza solo Zancudo");
    }

    Juego juego;
    juego.ventana.setVisible(false);
    juego._estado = Juego::EstadoJuego::Jugando;
    auto& horda = juego._gestorOleadas.obtenerEnemigos();
    horda.clear();
    const Posicion jugador = juego.jugador.getPosicion();
    for (int i = 0; i < 8; ++i)
    {
        const float angulo = i*3.14159265f/4;
        horda.push_back(crearEnemigo(RAPIDO, jugador.x+std::sin(angulo)*300-20,
                                    jugador.y+std::cos(angulo)*230-20));
    }
    for (int i = 0; i < 4; ++i)
    {
        horda.push_back(crearEnemigo(BASICO, jugador.x+(i%2 ? 400 : -400)-28,
                                    jugador.y+(i/2 ? 150 : -150)-28));
    }
    juego._aspectoEnemigos.actualizar(horda, juego.jugador.getFormaColision(), 0.025f);
    juego.relojFotogramas.restart();
    sf::sleep(sf::milliseconds(25));
    juego.actualizar();
    correcto &= comprobar(juego._aspectoEnemigos._estados.size() == 12,
                          "el juego sincroniza una horda mixta");
    const auto estadoPausa = juego._aspectoEnemigos._estados;
    juego._estado = Juego::EstadoJuego::Pausa;
    juego.actualizar();
    for (const auto& entrada : estadoPausa)
    {
        correcto &= comprobar(juego._aspectoEnemigos._estados.at(entrada.first).fase == entrada.second.fase,
                              "pausa integrada congela ambas variantes");
    }
    juego._estado = Juego::EstadoJuego::Jugando;
    juego._estacionMejoras._menuAbierto = true;
    juego.actualizar();
    for (const auto& entrada : estadoPausa)
    {
        correcto &= comprobar(juego._aspectoEnemigos._estados.at(entrada.first).fase == entrada.second.fase,
                              "menu integrado congela ambas variantes");
    }
    juego._estacionMejoras._menuAbierto = false;
    for (int cuadro = 0; cuadro < 32; ++cuadro)
    {
        juego.actualizarEnemigos(0.02f);
        juego._aspectoEnemigos.actualizar(horda, juego.jugador.getFormaColision(), 0.02f);
        juego.dibujar();
        sf::Texture captura(juego.ventana.getSize());
        captura.update(juego.ventana);
        if (!captura.copyToImage().saveToFile(salida / ("juego_"+std::to_string(cuadro)+".png")))
        {
            return 1;
        }
    }
    juego.reiniciarPrueba();
    correcto &= comprobar(juego._aspectoEnemigos._estados.empty(), "reinicio integrado retira ambas variantes");
    std::cout << (correcto ? "ASPECTO_ZANCUDO_OK\n" : "ASPECTO_ZANCUDO_FALLO\n");
    return correcto ? 0 : 1;
}
