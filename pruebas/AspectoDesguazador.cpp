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
    sf::RenderTexture superficie({512, 512});
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

int cantidadJefes(const std::vector<Enemigo>& enemigos)
{
    return static_cast<int>(std::count_if(enemigos.begin(), enemigos.end(),
        [](const Enemigo& enemigo) { return enemigo.obtenerIdEnemigo() == 4; }));
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
    const auto salida = raiz/"bin/Debug/desguazador_revision";
    std::filesystem::create_directories(salida);
    const std::array<std::string, 4> carpetas{{
        (raiz/"recursos/enemigos/mordaza_atlas").string(),
        (raiz/"recursos/enemigos/zancudo_atlas").string(),
        (raiz/"recursos/enemigos/yunque_atlas").string(),
        (raiz/"recursos/enemigos/desguazador_atlas").string()}};
    const std::string ausente = (salida/"ausente").string();
    AspectoEnemigos aspecto;
    for (int id = 1; id <= 4; ++id)
    {
        if (!comprobar(aspecto.animacionDisponible(id), "paquete de cada variante disponible"))
        {
            return 1;
        }
        correcto &= comprobar(aspecto._paquetes[id-1].celda == (id == 4 ? 256 : 192),
                              "celda correspondiente al jefe o al enemigo comun");
        for (const sf::Texture& textura : aspecto._paquetes[id-1].texturas)
        {
            correcto &= comprobar(!textura.isSmooth(), "sin suavizado de sprites");
        }
    }
    correcto &= comprobar(!aspecto.animacionDisponible(0) && !aspecto.animacionDisponible(5),
                          "identificadores fuera del arreglo seguros");
    const FormaColision lejos{2000, 2000, 48, 48};
    const std::vector<Enemigo> mezcla{crearEnemigo(BASICO, 100, 100),
        crearEnemigo(RAPIDO, 100, 100), crearEnemigo(RESISTENTE, 100, 100), crearEnemigo(JEFE, 100, 100)};
    sf::RenderTexture superficie({512, 512});
    for (int faltante = 0; faltante < 4; ++faltante)
    {
        auto rutas = carpetas;
        rutas[faltante] = ausente;
        AspectoEnemigos parcial(rutas[0].c_str(), rutas[1].c_str(), rutas[2].c_str(), rutas[3].c_str());
        parcial.actualizar(mezcla, lejos, .025f);
        correcto &= comprobar(parcial._estados.size() == 3, "paquete ausente no deja estado huerfano");
        for (int indice = 0; indice < 4; ++indice)
        {
            correcto &= comprobar(parcial.animacionDisponible(indice+1) == (indice != faltante) &&
                                  parcial.dibujar(superficie, mezcla[indice]) == (indice != faltante),
                                  "carga y respaldo independientes de las cuatro variantes");
        }
    }
    {
        AspectoEnemigos cruzado(carpetas[0].c_str(), carpetas[1].c_str(), carpetas[2].c_str(), carpetas[2].c_str());
        correcto &= comprobar(!cruzado.animacionDisponible(4) && cruzado.animacionDisponible(3),
                              "jefe rechaza cabecera de enemigo comun");
    }
    const auto invalido = salida/("atlas_invalido_"+
        std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::filesystem::create_directories(invalido);
    std::filesystem::copy_file(std::filesystem::path(carpetas[3])/"animacion.txt", invalido/"animacion.txt");
    sf::Image truncada({768, 1344}, sf::Color::White);
    if (!truncada.saveToFile(invalido/"cuerpo_0.png"))
    {
        return 1;
    }
    {
        AspectoEnemigos dimensiones(carpetas[0].c_str(), carpetas[1].c_str(), carpetas[2].c_str(), invalido.string().c_str());
        correcto &= comprobar(!dimensiones.animacionDisponible(4) && dimensiones.animacionDisponible(1),
                              "jefe rechaza textura con dimensiones de celda comun");
    }
    std::vector<Enemigo> enemigos = mezcla;
    const auto id = enemigos[3].obtenerIdInstancia();
    aspecto.actualizar(enemigos, lejos, .025f);
    const sf::Image reposo = representar(aspecto, enemigos[3]);
    enemigos[3].actualizar({2000, 100}, .025f);
    aspecto.actualizar(enemigos, lejos, .025f);
    const auto forma = enemigos[3].obtenerFormaColision();
    correcto &= comprobar(std::abs(forma.x-102.125f) < .001f && forma.ancho == 120 && forma.alto == 120 &&
                          enemigos[3].obtenerDano() == 35, "velocidad 85, colision 120 y dano 35 conservados");
    correcto &= comprobar(aspecto._estados.at(id).fase > 0 &&
                          !iguales(reposo, representar(aspecto, enemigos[3])), "marcha cambia pose del jefe");
    const std::array<Posicion, 8> movimientos{{{0, 1}, {1, 1}, {1, 0}, {1, -1},
                                            {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}}};
    for (int direccion = 0; direccion < 8; ++direccion)
    {
        const Posicion p = enemigos[3].obtenerPosicion();
        enemigos[3].actualizar({p.x+movimientos[direccion].x*100,
                               p.y+movimientos[direccion].y*100}, .025f);
        aspecto.actualizar(enemigos, lejos, .025f);
        correcto &= comprobar(aspecto._estados.at(id).direccion == direccion, "jefe gira en ocho vistas");
    }
    aspecto.actualizar(enemigos, lejos, .025f);
    const float fase = aspecto._estados.at(id).fase;
    correcto &= comprobar(!aspecto._estados.at(id).enMovimiento, "reposo detiene marcha");
    const auto quieto = representar(aspecto, enemigos[3]);
    aspecto.actualizar(enemigos, lejos, 0);
    aspecto.actualizar(enemigos, lejos, std::numeric_limits<float>::quiet_NaN());
    correcto &= comprobar(aspecto._estados.at(id).fase == fase &&
                          iguales(quieto, representar(aspecto, enemigos[3])), "pausa conserva pose");
    aspecto.actualizar(enemigos, enemigos[3].obtenerFormaColision(), .025f);
    const auto contacto = representar(aspecto, enemigos[3]);
    aspecto.actualizar(enemigos, enemigos[3].obtenerFormaColision(), .075f);
    correcto &= comprobar(!iguales(contacto, representar(aspecto, enemigos[3])), "contacto articula pinza y prensa");
    enemigos[3].recibirDanio(2999);
    correcto &= comprobar(enemigos[3].estaVivo(), "vida del jefe sigue siendo 3000");
    enemigos[3].recibirDanio(1);
    aspecto.actualizar(enemigos, lejos, .025f);
    correcto &= comprobar(aspecto._estados.count(id) == 0 && aspecto._estados.size() == 3,
                          "muerte retira solo el aspecto del jefe");
    aspecto.reiniciar();
    correcto &= comprobar(aspecto._estados.empty(), "reinicio limpia los cuatro tipos");

    Juego juego;
    juego.ventana.setVisible(false);
    juego._estado = Juego::EstadoJuego::Jugando;
    const auto jugador = juego.jugador.getPosicion();
    auto& gestor = juego._gestorOleadas;
    auto& horda = gestor.obtenerEnemigos();
    gestor.actualizar(jugador, 299.f);
    correcto &= comprobar(!gestor.jefeEstaVivo() && cantidadJefes(horda) == 0,
                          "no hay jefe antes del segundo 300");
    gestor.actualizar(jugador, 1.f);
    if (!comprobar(gestor.jefeEstaVivo() && cantidadJefes(horda) == 1,
                   "el jefe real aparece al segundo 300"))
    {
        return 1;
    }
    horda.erase(std::remove_if(horda.begin(), horda.end(),
        [](const Enemigo& enemigo) { return enemigo.obtenerIdEnemigo() != 4; }), horda.end());
    const auto idJefeReal = horda[0].obtenerIdInstancia();
    horda[0].actualizar({jugador.x-230-60, jugador.y-170-60}, 20.f);
    horda.push_back(crearEnemigo(BASICO, jugador.x+300-28, jugador.y-100-28));
    horda.push_back(crearEnemigo(RAPIDO, jugador.x+440-20, jugador.y+80-20));
    horda.push_back(crearEnemigo(RESISTENTE, jugador.x+230-36, jugador.y+190-36));
    juego._partida.actualizar(300.f);
    juego._aspectoEnemigos.actualizar(horda, juego.jugador.getFormaColision(), .025f);
    correcto &= comprobar(juego._aspectoEnemigos._estados.size() == 4,
                          "el jefe generado utiliza su sprite junto a los tres enemigos");
    const auto estados = juego._aspectoEnemigos._estados;
    juego._estado = Juego::EstadoJuego::Pausa;
    juego.actualizar();
    juego._estado = Juego::EstadoJuego::Jugando;
    juego._estacionMejoras._menuAbierto = true;
    juego.actualizar();
    for (const auto& entrada : estados)
    {
        correcto &= comprobar(juego._aspectoEnemigos._estados.at(entrada.first).fase == entrada.second.fase,
                              "pausa y estacion congelan todos los aspectos");
    }
    juego._estacionMejoras._menuAbierto = false;
    for (int cuadro = 0; cuadro < 32; ++cuadro)
    {
        juego.actualizarEnemigos(.025f);
        juego._aspectoEnemigos.actualizar(horda, juego.jugador.getFormaColision(), .025f);
        juego.dibujar();
        sf::Texture captura(juego.ventana.getSize());
        captura.update(juego.ventana);
        if (!captura.copyToImage().saveToFile(salida/("juego_"+std::to_string(cuadro)+".png")))
        {
            return 1;
        }
    }
    correcto &= comprobar(cantidadJefes(horda) == 1, "no se duplica el jefe en fotogramas siguientes");
    for (Enemigo& enemigo : horda)
    {
        if (enemigo.obtenerIdInstancia() == idJefeReal)
        {
            enemigo.recibirDanio(3000);
        }
    }
    juego._aspectoEnemigos.actualizar(horda, juego.jugador.getFormaColision(), .025f);
    correcto &= comprobar(juego._aspectoEnemigos._estados.count(idJefeReal) == 0, "jefe muerto deja de dibujarse");
    juego.actualizarPartida(0);
    correcto &= comprobar(juego._partida.fueVictoria() && juego._partidaGuardada,
                          "muerte del jefe conserva victoria y guardado de resultado");
    gestor.actualizar(jugador, 1.f);
    correcto &= comprobar(gestor.jefeFueDerrotado() && cantidadJefes(horda) == 0,
                          "jefe derrotado se retira y no reaparece");
    juego.reiniciarPrueba();
    correcto &= comprobar(juego._aspectoEnemigos._estados.empty() && !gestor.jefeEstaVivo(),
                          "reinicio restaura gestor y todos los aspectos");
    std::cout << (correcto ? "ASPECTO_DESGUAZADOR_OK\n" : "ASPECTO_DESGUAZADOR_FALLO\n");
    return correcto ? 0 : 1;
}
