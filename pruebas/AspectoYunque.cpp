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
    sf::RenderTexture superficie({320, 320});
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
    const auto salida = raiz / "bin/Debug/yunque_revision";
    std::filesystem::create_directories(salida);
    const std::array<std::string, 3> carpetas{{
        (raiz/"recursos/enemigos/mordaza_atlas").string(),
        (raiz/"recursos/enemigos/zancudo_atlas").string(),
        (raiz/"recursos/enemigos/yunque_atlas").string()}};
    const std::string ausente = (salida/"ausente").string();
    AspectoEnemigos aspecto;
    if (!comprobar(aspecto.animacionDisponible(1) && aspecto.animacionDisponible(2) &&
                   aspecto.animacionDisponible(3), "los tres paquetes disponibles"))
    {
        return 1;
    }
    correcto &= comprobar(!aspecto.animacionDisponible(0) && !aspecto.animacionDisponible(5) &&
                          !aspecto.animacionDisponible(-1), "limites de identificadores seguros");
    for (const auto& paquete : aspecto._paquetes)
    {
        for (const sf::Texture& textura : paquete.texturas)
        {
            correcto &= comprobar(!textura.isSmooth(), "los tres atlas sin suavizado");
        }
    }
    sf::RenderTexture superficie({320, 320});
    const std::vector<Enemigo> mezcla{crearEnemigo(BASICO, 100, 100),
        crearEnemigo(RAPIDO, 100, 100), crearEnemigo(RESISTENTE, 100, 100)};
    const FormaColision lejos{1000, 1000, 48, 48};
    for (int faltante = 0; faltante < 3; ++faltante)
    {
        auto rutas = carpetas;
        rutas[faltante] = ausente;
        AspectoEnemigos parcial(rutas[0].c_str(), rutas[1].c_str(), rutas[2].c_str());
        parcial.actualizar(mezcla, lejos, .025f);
        correcto &= comprobar(parcial._estados.size() == 2, "recurso ausente no crea estados huerfanos");
        for (int indice = 0; indice < 3; ++indice)
        {
            correcto &= comprobar(parcial.animacionDisponible(indice+1) == (indice != faltante),
                                  "fallo de un paquete no desactiva los otros dos");
            correcto &= comprobar(parcial.dibujar(superficie, mezcla[indice]) == (indice != faltante),
                                  "respaldo independiente al dibujar");
        }
    }
    {
        AspectoEnemigos cabeceraCruzada(carpetas[0].c_str(), carpetas[1].c_str(), carpetas[0].c_str());
        correcto &= comprobar(cabeceraCruzada.animacionDisponible(1) &&
            cabeceraCruzada.animacionDisponible(2) && !cabeceraCruzada.animacionDisponible(3),
            "Yunque rechaza cabecera de Mordaza sin desactivar las otras variantes");
    }
    std::vector<Enemigo> enemigos = mezcla;
    aspecto.actualizar(enemigos, lejos, .025f);
    const auto id = enemigos[2].obtenerIdInstancia();
    const sf::Image reposo = representar(aspecto, enemigos[2]);
    for (Enemigo& enemigo : enemigos)
    {
        enemigo.actualizar({2000, 100}, .025f);
    }
    aspecto.actualizar(enemigos, lejos, .025f);
    correcto &= comprobar(aspecto._estados.size() == 3 && aspecto._estados.at(id).fase > 0,
                          "horda de tres tipos mantiene estados y avanza marcha");
    const FormaColision forma = enemigos[2].obtenerFormaColision();
    correcto &= comprobar(std::abs(forma.x-101.75f) < .001f && forma.ancho == 72 && forma.alto == 72 &&
                          enemigos[2].obtenerDano() == 15, "velocidad 70, colision 72 y dano 15 conservados");
    correcto &= comprobar(!iguales(reposo, representar(aspecto, enemigos[2])), "marcha cambia la pose de Yunque");
    const std::array<Posicion, 8> movimientos{{{0, 1}, {1, 1}, {1, 0}, {1, -1},
                                            {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}}};
    for (int direccion = 0; direccion < 8; ++direccion)
    {
        const Posicion p = enemigos[2].obtenerPosicion();
        enemigos[2].actualizar({p.x+movimientos[direccion].x*100,
                               p.y+movimientos[direccion].y*100}, .025f);
        aspecto.actualizar(enemigos, lejos, .025f);
        correcto &= comprobar(aspecto._estados.at(id).direccion == direccion, "ocho vistas de Yunque");
    }
    const float fase = aspecto._estados.at(id).fase;
    aspecto.actualizar(enemigos, lejos, .025f);
    correcto &= comprobar(!aspecto._estados.at(id).enMovimiento && aspecto._estados.at(id).fase == fase,
                          "reposo detiene patas y conserva fase");
    const sf::Image quieto = representar(aspecto, enemigos[2]);
    aspecto.actualizar(enemigos, lejos, 0);
    aspecto.actualizar(enemigos, lejos, std::numeric_limits<float>::quiet_NaN());
    correcto &= comprobar(iguales(quieto, representar(aspecto, enemigos[2])), "tiempo invalido no cambia pose");
    aspecto.actualizar(enemigos, enemigos[2].obtenerFormaColision(), .025f);
    const sf::Image contacto = representar(aspecto, enemigos[2]);
    aspecto.actualizar(enemigos, enemigos[2].obtenerFormaColision(), .075f);
    correcto &= comprobar(!iguales(contacto, representar(aspecto, enemigos[2])), "contacto comprime suspension y frente");
    enemigos[2].recibirDanio(249);
    correcto &= comprobar(enemigos[2].estaVivo(), "vida de resistente sigue siendo 250");
    enemigos[2].recibirDanio(1);
    aspecto.actualizar(enemigos, lejos, .025f);
    correcto &= comprobar(!enemigos[2].estaVivo() && aspecto._estados.count(id) == 0 &&
                          aspecto._estados.size() == 2, "muerte limpia solo al resistente");
    enemigos[2] = crearEnemigo(RESISTENTE, 100, 100);
    aspecto.actualizar(enemigos, lejos, .025f);
    correcto &= comprobar(aspecto._estados.at(enemigos[2].obtenerIdInstancia()).fase == 0,
                          "nueva instancia empieza con fase limpia");
    aspecto.reiniciar();
    correcto &= comprobar(aspecto._estados.empty(), "reinicio limpia los tres tipos");
    correcto &= comprobar(!aspecto.dibujar(superficie,
        Enemigo(99, 100, 100, 56, 56, 0, 100, 10, "Desconocido", 5, 3)),
        "variante desconocida conserva respaldo");
    const auto invalido = salida / ("atlas_invalido_"+
        std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::filesystem::create_directories(invalido);
    std::filesystem::copy_file(std::filesystem::path(carpetas[2])/"animacion.txt", invalido/"animacion.txt");
    sf::Image truncada({768, 1343}, sf::Color::White);
    if (!truncada.saveToFile(invalido/"cuerpo_0.png"))
    {
        return 1;
    }
    {
        AspectoEnemigos dimensiones(carpetas[0].c_str(), carpetas[1].c_str(), invalido.string().c_str());
        correcto &= comprobar(dimensiones.animacionDisponible(1) && dimensiones.animacionDisponible(2) &&
                              !dimensiones.animacionDisponible(3), "dimensiones invalidas rechazan solo Yunque");
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
        horda.push_back(crearEnemigo(RESISTENTE, jugador.x+std::sin(angulo)*310-36,
                                    jugador.y+std::cos(angulo)*235-36));
    }
    for (int i = 0; i < 4; ++i)
    {
        const bool rapido = i/2 == 0;
        const float mitad = rapido ? 20.f : 28.f;
        horda.push_back(crearEnemigo(rapido ? RAPIDO : BASICO,
            jugador.x+(i%2 ? 460 : -460)-mitad, jugador.y+(rapido ? -110 : 150)-mitad));
    }
    juego._aspectoEnemigos.actualizar(horda, juego.jugador.getFormaColision(), .025f);
    juego.relojFotogramas.restart();
    sf::sleep(sf::milliseconds(25));
    juego.actualizar();
    correcto &= comprobar(juego._aspectoEnemigos._estados.size() == 12, "actualizacion integra los tres aspectos");
    const auto estados = juego._aspectoEnemigos._estados;
    juego._estado = Juego::EstadoJuego::Pausa;
    juego.actualizar();
    for (const auto& entrada : estados)
    {
        correcto &= comprobar(juego._aspectoEnemigos._estados.at(entrada.first).fase == entrada.second.fase,
                              "pausa integrada congela los tres tipos");
    }
    juego._estado = Juego::EstadoJuego::Jugando;
    juego._estacionMejoras._menuAbierto = true;
    juego.actualizar();
    for (const auto& entrada : estados)
    {
        correcto &= comprobar(juego._aspectoEnemigos._estados.at(entrada.first).fase == entrada.second.fase,
                              "estacion congela los tres tipos");
    }
    juego._estacionMejoras._menuAbierto = false;
    for (int cuadro = 0; cuadro < 32; ++cuadro)
    {
        juego.actualizarEnemigos(.025f);
        juego._aspectoEnemigos.actualizar(horda, juego.jugador.getFormaColision(), .025f);
        juego.dibujar();
        sf::Texture captura(juego.ventana.getSize());
        captura.update(juego.ventana);
        if (!captura.copyToImage().saveToFile(salida / ("juego_"+std::to_string(cuadro)+".png")))
        {
            return 1;
        }
    }
    juego.reiniciarPrueba();
    correcto &= comprobar(juego._aspectoEnemigos._estados.empty(), "reinicio integrado vacia todos los estados");
    std::cout << (correcto ? "ASPECTO_YUNQUE_OK\n" : "ASPECTO_YUNQUE_FALLO\n");
    return correcto ? 0 : 1;
}
